#include "GstAHardwareBufferVideoBuffer.h"

#if defined(QGC_HAS_GST_AHARDWAREBUFFER_GPU_PATH)

#include <android/hardware_buffer.h>
#include <array>
#include <atomic>
#include <gst/android/gstandroid.h>
#include <gst/gl/gstglsyncmeta.h>
#include <gst/video/video.h>
#include <limits>
#include <private/qvideotexturehelper_p.h>
#include <rhi/qrhi.h>
#include <rhi/qrhi_platform.h>

#include <QtCore/QLoggingCategory>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QScopeGuard>
#include <QtCore/QSize>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>

#include "GstContextBridgeRegistry.h"
#include "GstEglHelpers.h"
#include "GstHwFrameTexturesBase.h"
#include "GstHwImportCache.h"
#include "GstHwImportPreflight.h"
#include "GstHwPathTelemetry.h"
#include "GstHwVideoBuffer.h"
#include "QGCLoggingCategory.h"
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

QGC_LOGGING_CATEGORY(GstAHWBufLog, "Video.GStreamer.HwBuffers.GstAHWBuf")

namespace {

using GstHw::kMaxPlanes;

std::atomic<bool> s_loggedBadBackend{false};

constexpr const char* kNativeBufferExt = "EGL_ANDROID_image_native_buffer";

std::atomic<bool> s_loggedNoFenceSync{false};

PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageFn()
{
    static const auto fn = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    return fn;
}

// LRU cache (QRhi, AHardwareBuffer, EGLDisplay) -> (EGLImage, GL texture): MediaCodec recycles the same AHB so the
// import stays valid, saving ~50-200 us/frame on Mali/Adreno.
struct AhbCacheKey
{
    QRhi* rhi = nullptr;
    AHardwareBuffer* ahb = nullptr;
    EGLDisplay display = EGL_NO_DISPLAY;

    bool operator==(const AhbCacheKey&) const noexcept = default;
};

struct AhbImport
{
    QOpenGLContext* context = nullptr;  // context the texture name was generated in
    AHardwareBuffer* ahb = nullptr;     // acquired ref
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLImageKHR image = EGL_NO_IMAGE_KHR;
    GLuint textureName = 0;
};

void destroyAhbImport(AhbImport& e)
{
    if (e.image != EGL_NO_IMAGE_KHR && e.display != EGL_NO_DISPLAY && eglDestroyImageFn()) {
        eglDestroyImageFn()(e.display, e.image);
    }
    if (e.textureName != 0) {
        if (QOpenGLContext* ctx = QOpenGLContext::currentContext()) {
            ctx->functions()->glDeleteTextures(1, &e.textureName);
        }
    }
    if (e.ahb) {
        AHardwareBuffer_release(e.ahb);
    }
}

// glDeleteTextures off the owning context leaks the name (or deletes another context's), so entries removed elsewhere
// (a reset from a non-render thread) are parked until that context is current again.
bool ownerContextCurrent(const AhbImport& e)
{
    return QOpenGLContext::currentContext() == e.context;
}

constexpr std::size_t kImageCacheCapacity = 4;
using AhbImportCache = GstHw::GstHwImportCache<AhbCacheKey, AhbImport, kImageCacheCapacity>;

// Live FrameTextures pin the entry whose texture they wrap, so eviction or reset never frees a name still sampled.
QMutex s_imageCacheMutex;
AhbImportCache s_imageCache{destroyAhbImport, ownerContextCurrent};

// QRhi cleanup callback (runs in ~QRhi on its render thread while its GL context still exists, after Qt destroyed the
// frame bundles holding pins): drop every entry keyed on @p rhi, so a QRhi later allocated at the same address can
// never hit a texture name from the dead context.
void purgeImageCacheForRhi(QRhi* rhi)
{
    rhi->makeThreadLocalNativeContextCurrent();
    const QMutexLocker lock(&s_imageCacheMutex);
    s_imageCache.eraseIf([rhi](const AhbCacheKey& key, const AhbImport&) { return key.rhi == rhi; });
    s_imageCache.drainDeferred();
}

class FrameTextures final : public GstHwFrameTexturesBase
{
public:
    // Always single-plane external-OES; the GL texture name is owned by the process-wide import cache entry @p entry,
    // which this bundle pins (shared across frames on AHB recycle).
    FrameTextures(QRhi* rhi, QSize size, QVideoFrameFormat::PixelFormat pixelFormat, GLuint name,
                  AhbImportCache::EntryId entry)
        : _rhi(rhi)
        , _size(size)
        , _pixelFormat(pixelFormat)
        , _name(name)
        , _entry(entry)
    {
        {
            const QMutexLocker lock(&s_imageCacheMutex);
            s_imageCache.pin(_entry);
        }
        _count = 1;
        const auto* desc = QVideoTextureHelper::textureDescription(pixelFormat);
        if (!desc) {
            qCWarning(GstAHWBufLog) << "no QVideoTextureHelper description for format" << pixelFormat;
            return;
        }
        const QSize planeSize = desc->rhiPlaneSize(size, 0, rhi);
        // ExternalOES: bind GL_TEXTURE_EXTERNAL_OES + emit SamplerExternalOES, else OES_EGL_image_external samples
        // black on GLES2.
        _textures[0].reset(rhi->newTexture(
            desc->rhiTextureFormat(0, rhi, QVideoTextureHelper::TextureDescription::FallbackPolicy::Disable), planeSize,
            1, QRhiTexture::ExternalOES));
        if (_textures[0] && !_textures[0]->createFrom({name, 0})) {
            qCWarning(GstAHWBufLog) << "QRhiTexture::createFrom failed for AHardwareBuffer plane 0";
            _textures[0].reset();
        }
    }

    ~FrameTextures() override
    {
        if (_rhi) {
            _rhi->makeThreadLocalNativeContextCurrent();
        }
        const QMutexLocker lock(&s_imageCacheMutex);
        s_imageCache.unpin(_entry);
        s_imageCache.drainDeferred();
    }

    HwVideoBufferPath sourcePath() const override { return HwVideoBufferPath::AHardwareBuffer; }

    // Reuse eligibility: same rhi+size+format and identical GL texture name (AhbCache returns the same name on AHB
    // recycle).
    bool matches(QRhi* rhi, QSize size, QVideoFrameFormat::PixelFormat pixelFormat, GLuint name) const noexcept
    {
        return _rhi == rhi && _size == size && _pixelFormat == pixelFormat && _name == name && _name != 0 &&
               _textures[0];
    }

private:
    QRhi* _rhi = nullptr;
    QSize _size;
    QVideoFrameFormat::PixelFormat _pixelFormat = QVideoFrameFormat::Format_Invalid;
    GLuint _name = 0;
    AhbImportCache::EntryId _entry = 0;
};

// MediaCodec writes the AHB asynchronously; without a producer-side wait the EGLImage import can sample a half-written,
// recycled buffer (tearing). MediaCodec AHB buffers carry no GstGLSyncMeta and the gstandroid memory API exposes no
// acquire fence, so in practice no producer wait happens here; a sync meta is only present when a gst-gl element
// produced the buffer. A consumer-side EGL fence cannot observe MediaCodec's write, so none is used.
void waitProducerComplete(GstBuffer* buffer)
{
    if (GstGLSyncMeta* syncMeta = buffer ? gst_buffer_get_gl_sync_meta(buffer) : nullptr) {
        if (GstGLContext* glObj = syncMeta->context) {
            // CPU wait: gst_gl_sync_meta_wait() would only order the producer's own context, not Qt's.
            gst_gl_sync_meta_wait_cpu(syncMeta, glObj);
            return;
        }
    }

    QGC_HW_WARN_ONCE(GstAHWBufLog, s_loggedNoFenceSync,
                     "AHardwareBuffer: no GstGLSyncMeta producer fence; frames may tear on recycle (no consumer-side "
                     "barrier can substitute)");
}

}  // namespace

GstAHardwareBufferVideoBuffer::GstAHardwareBufferVideoBuffer(GstSample* sample, const GstVideoInfo& videoInfo,
                                                             const QVideoFrameFormat& format, EGLDisplay eglDisplay)
    : GstHwVideoBuffer(QVideoFrame::RhiTextureHandle, sample, videoInfo, format), _eglDisplay(eglDisplay)
{}

// The AHB is imported directly as an EGLImage (no SurfaceTexture::updateTexImage()), so no per-buffer
// getTransformMatrix() exists here; external-OES sampling of a directly-imported buffer is top-left origin while GL
// texcoords are bottom-left, so apply the static Y-flip Qt's SurfaceTexture path also applies (flipV, row-major).
QMatrix4x4 GstAHardwareBufferVideoBuffer::externalTextureMatrix() const
{
    // GStreamer exposes no per-buffer decoder transform at this layer, so we apply the static external-OES Y-flip
    // (top-left vs GL bottom-left origin). If a device ever needs crop-inset/rotation, source it from GstVideoCropMeta.
    static const QMatrix4x4 flipV(1.0f, 0.0f, 0.0f, 0.0f,
                                  0.0f, -1.0f, 0.0f, 1.0f,
                                  0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f);
    return flipV;
}

bool GstAHardwareBufferVideoBuffer::validatePlaneHandles() const
{
    // Qt 6.10-6.12 has the right ExternalOES rendering path, but upstream GStreamer still does not expose this
    // AHardwareBuffer memory API. If a future GStreamer/Qt release grows an official API, replace these vendor hooks.
    return validatePlanes([](GstMemory* mem) {
        if (!mem || !gst_is_ahardware_buffer_memory(mem))
            return false;
        return gst_ahardware_buffer_memory_get_buffer(GST_AHARDWARE_BUFFER_MEMORY_CAST(mem)) != nullptr;
    });
}

// Callable from any thread: entries whose context is not current here are destroyed on its render thread later.
void GstAHardwareBufferVideoBuffer::resetImageCache() noexcept
{
    const QMutexLocker lock(&s_imageCacheMutex);
    s_imageCache.clear();
    s_imageCache.drainDeferred();
}

namespace {
struct AhbCacheResetRegistrar
{
    AhbCacheResetRegistrar() { GstContextBridgeRegistry::registerCacheReset(&GstAHardwareBufferVideoBuffer::resetImageCache); }
};
const AhbCacheResetRegistrar s_ahbCacheResetRegistrar;
}  // namespace

QVideoFrameTexturesUPtr GstAHardwareBufferVideoBuffer::mapTextures(QRhi& rhi, QVideoFrameTexturesUPtr& old)
{
    // Qt's contract: mapTextures runs on the QRhi (render) thread. Bail rather than crash if ever called off-thread.
    if (!rhi.thread()->isCurrentThread()) {
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }

    if (!_sample) {
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }
    if (rhi.backend() != QRhi::OpenGLES2) {
        if (!s_loggedBadBackend.exchange(true, std::memory_order_relaxed)) {
            qCWarning(GstAHWBufLog) << "QRhi backend is not OpenGLES2; AHardwareBuffer path unsupported";
        }
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }

    // Bind Qt's GL context before any EGL/GL call, else the calls silently no-op into a foreign/null context.
    rhi.makeThreadLocalNativeContextCurrent();

    const auto* nativeHandles = static_cast<const QRhiGles2NativeHandles*>(rhi.nativeHandles());
    if (!nativeHandles || !nativeHandles->context) {
        qCWarning(GstAHWBufLog) << "QRhi exposes no GL context";
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }

    // Prefer QEGLContext::display() — sampling on a foreign display silently returns black.
    EGLDisplay eglDpy = GstEglHelpers::resolveEglDisplay(nativeHandles->context);
    if (eglDpy == EGL_NO_DISPLAY)
        eglDpy = _eglDisplay;
    if (eglDpy == EGL_NO_DISPLAY) {
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }

    GstBuffer* buffer = gst_sample_get_buffer(_sample);
    if (!buffer)
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);

    GstMemory* mem0 = gst_buffer_peek_memory(buffer, 0);
    if (!mem0 || !gst_is_ahardware_buffer_memory(mem0)) {
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }

    if (!GstEglHelpers::displaySupportsExtension(eglDpy, kNativeBufferExt)) {
        static std::atomic<bool> s_warnedNativeBufferExt{false};
        if (!s_warnedNativeBufferExt.exchange(true, std::memory_order_relaxed)) {
            qCWarning(GstAHWBufLog) << "EGL_ANDROID_image_native_buffer unavailable; AHardwareBuffer path disabled";
        }
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }

    static const auto eglGetNativeClientBufferANDROID_ =
        reinterpret_cast<PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC>(eglGetProcAddress("eglGetNativeClientBufferANDROID"));
    static const auto eglCreateImageKHR_ =
        reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    const auto eglDestroyImageKHR_ = eglDestroyImageFn();
    static const auto glEGLImageTargetTexture2DOES_ =
        reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));

    if (!eglGetNativeClientBufferANDROID_ || !eglCreateImageKHR_ || !eglDestroyImageKHR_ ||
        !glEGLImageTargetTexture2DOES_) {
        qCWarning(GstAHWBufLog) << "Required EGL/GL proc addresses unavailable";
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }

    AHardwareBuffer* ahwb = gst_ahardware_buffer_memory_get_buffer(GST_AHARDWARE_BUFFER_MEMORY_CAST(mem0));
    if (!ahwb) {
        qCWarning(GstAHWBufLog) << "gst_ahardware_buffer_memory_get_buffer returned null";
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }

    // Block until the producer's GPU write completes before importing/sampling the recycled AHB.
    waitProducerComplete(buffer);

    QOpenGLFunctions functions(nativeHandles->context);

    const AhbCacheKey key{&rhi, ahwb, eglDpy};
    GLuint name = 0;
    AhbImportCache::EntryId entry = 0;
    {
        const QMutexLocker lock(&s_imageCacheMutex);
        if (const auto hit = s_imageCache.find(key)) {
            name = hit.resource->textureName;
            entry = hit.id;
            s_imageCache.pin(entry);  // provisional pin: hold the entry across the unlocked gap below
            GstHwPathTelemetry::recordImageCacheHit(HwVideoBufferPath::AHardwareBuffer);
        }
    }

    if (name == 0) {
        GstHwPathTelemetry::recordImageCacheMiss(HwVideoBufferPath::AHardwareBuffer);

        EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID_(ahwb);
        if (!clientBuffer) {
            qCWarning(GstAHWBufLog) << "eglGetNativeClientBufferANDROID returned null";
            return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
        }

        const EGLint attribs[] = {EGL_NONE};
        EGLImageKHR image =
            eglCreateImageKHR_(eglDpy, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, attribs);
        if (image == EGL_NO_IMAGE_KHR) {
            qCWarning(GstAHWBufLog) << "eglCreateImageKHR failed, err=" << Qt::hex << eglGetError();
            return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
        }

        functions.glGenTextures(1, &name);
        functions.glBindTexture(GL_TEXTURE_EXTERNAL_OES, name);
        glEGLImageTargetTexture2DOES_(GL_TEXTURE_EXTERNAL_OES, image);
        if (Q_UNLIKELY(GstAHWBufLog().isDebugEnabled())) {
            if (const GLenum glErr = functions.glGetError(); glErr != GL_NO_ERROR) {
                qCDebug(GstAHWBufLog) << "glEGLImageTargetTexture2DOES glError=" << Qt::hex << glErr
                                      << "eglError=" << eglGetError();
            }
        }

        const QMutexLocker lock(&s_imageCacheMutex);
        // Real race: s_imageCache is process-wide and each window's render thread can import the same recycled ahb, so
        // re-check and reuse theirs if present.
        if (const auto existing = s_imageCache.find(key)) {
            eglDestroyImageKHR_(eglDpy, image);
            functions.glDeleteTextures(1, &name);
            name = existing.resource->textureName;
            entry = existing.id;
        } else {
            AHardwareBuffer_acquire(ahwb);
            const auto inserted =
                s_imageCache.insert(key, AhbImport{nativeHandles->context, ahwb, eglDpy, image, name});
            if (inserted) {
                entry = inserted.id;
                // Keyed: registered once per QRhi however many entries it inserts.
                rhi.addCleanupCallback(&s_imageCache, purgeImageCacheForRhi);
            } else {
                // Every entry is pinned by a live frame; insert() already released this import.
                qCWarning(GstAHWBufLog) << "AHB texture cache full and all entries in use; skipping frame";
                name = 0;
            }
        }
        s_imageCache.pin(entry);  // provisional pin (no-op if entry == 0)
    }

    if (name == 0) {
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }

    // Provisional pin keeps LRU eviction from freeing the name before FrameTextures takes its own pin.
    const auto pinGuard = qScopeGuard([&] {
        const QMutexLocker lock(&s_imageCacheMutex);
        s_imageCache.unpin(entry);
        s_imageCache.drainDeferred();
    });

    if (auto* prev = GstHwFrameTexturesBase::reusableBundle<FrameTextures>(old, HwVideoBufferPath::AHardwareBuffer)) {
        if (prev->matches(&rhi, _format.frameSize(), QVideoFrameFormat::Format_SamplerExternalOES, name)) {
            GstHwPathTelemetry::recordTextureReuse(HwVideoBufferPath::AHardwareBuffer);
            prev->setSourceSample(takeSample());
            QVideoFrameTexturesUPtr reused = std::move(old);
            return reused;
        }
    }

    // Pre-flight the external-OES RHI format/size before createFrom() so an unsupported import demotes to CPU on a query.
    if (!GstHwImportPreflight::preflightOrRecord(&rhi, HwVideoBufferPath::AHardwareBuffer,
                                                 QVideoFrameFormat::Format_SamplerExternalOES, _format.frameSize(),
                                                 QRhiTexture::ExternalOES)) {
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }

    auto textures = std::make_unique<FrameTextures>(&rhi, _format.frameSize(),
                                                    QVideoFrameFormat::Format_SamplerExternalOES, name, entry);
    if (!textures->texture(0)) {
        qCWarning(GstAHWBufLog) << "createFrom failed for plane 0 (SamplerExternalOES)";
        return GstHwPathTelemetry::fail(HwVideoBufferPath::AHardwareBuffer);
    }
    textures->setSourceSample(takeSample());
    return textures;
}

#endif  // QGC_HAS_GST_AHARDWAREBUFFER_GPU_PATH
