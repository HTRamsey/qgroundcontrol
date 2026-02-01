#include "AndroidInterface.h"
#ifndef QGC_NO_SERIAL_LINK
    #include "AndroidSerial.h"
#endif
#include "QGCLoggingCategory.h"

#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#include <QtCore/QLoggingCategory>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

QGC_LOGGING_CATEGORY(AndroidInitLog, "Android.AndroidInit");

static jobject _context = nullptr;
static jobject _class_loader = nullptr;
static JavaVM *_java_vm = nullptr;

#ifdef QGC_GST_STREAMING
extern "C"
{
    extern void gst_amc_jni_set_java_vm(JavaVM *java_vm);

    // GStreamer Android helper functions
    // These are called by GStreamer's androidmedia plugins
    jobject gst_android_get_application_context(void)
    {
        return _context;
    }

    jobject gst_android_get_application_class_loader(void)
    {
        return _class_loader;
    }

    JavaVM *gst_android_get_java_vm(void)
    {
        return _java_vm;
    }
}

// Set up GStreamer environment variables for Android
// This replicates what gst_android_init() does in the GStreamer template
extern "C" void gst_android_setup_environment()
{
    // Get application directories using Qt
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (!cacheDir.isEmpty()) {
        qputenv("TMP", cacheDir.toUtf8());
        qputenv("TEMP", cacheDir.toUtf8());
        qputenv("TMPDIR", cacheDir.toUtf8());
        qputenv("XDG_RUNTIME_DIR", cacheDir.toUtf8());
        qputenv("XDG_CACHE_HOME", cacheDir.toUtf8());

        // GStreamer registry cache
        const QString registry = cacheDir + QStringLiteral("/gstreamer-registry.bin");
        qputenv("GST_REGISTRY", registry.toUtf8());
        qputenv("GST_REGISTRY_REUSE_PLUGIN_SCANNER", "no");

        qCDebug(AndroidInitLog) << "GStreamer cache dir:" << cacheDir;
    }

    if (!dataDir.isEmpty()) {
        qputenv("HOME", dataDir.toUtf8());
        qputenv("XDG_DATA_DIRS", dataDir.toUtf8());
        qputenv("XDG_CONFIG_DIRS", dataDir.toUtf8());
        qputenv("XDG_CONFIG_HOME", dataDir.toUtf8());
        qputenv("XDG_DATA_HOME", dataDir.toUtf8());

        // Fontconfig path (if fonts are deployed)
        const QString fontconfigDir = dataDir + QStringLiteral("/fontconfig");
        if (QDir(fontconfigDir).exists()) {
            qputenv("FONTCONFIG_PATH", fontconfigDir.toUtf8());
        }

        // SSL certificates (if deployed)
        const QString certsFile = dataDir + QStringLiteral("/ssl/certs/ca-certificates.crt");
        if (QFile::exists(certsFile)) {
            qputenv("CA_CERTIFICATES", certsFile.toUtf8());
            qCDebug(AndroidInitLog) << "GStreamer CA certificates:" << certsFile;
        }

        qCDebug(AndroidInitLog) << "GStreamer data dir:" << dataDir;
    }
}
#endif

static jboolean jniInit(JNIEnv *env, jobject context)
{
    qCDebug(AndroidInitLog) << Q_FUNC_INFO;

    const jclass context_cls = env->GetObjectClass(context);
    if (!context_cls) {
        return JNI_FALSE;
    }

    const jmethodID get_class_loader_id = env->GetMethodID(context_cls, "getClassLoader", "()Ljava/lang/ClassLoader;");
    if (QJniEnvironment::checkAndClearExceptions(env)) {
        return JNI_FALSE;
    }

    const jobject class_loader = env->CallObjectMethod(context, get_class_loader_id);
    if (QJniEnvironment::checkAndClearExceptions(env)) {
        return JNI_FALSE;
    }

    _context = env->NewGlobalRef(context);
    _class_loader = env->NewGlobalRef(class_loader);

    return JNI_TRUE;
}

static jint jniSetNativeMethods()
{
    qCDebug(AndroidInitLog) << Q_FUNC_INFO;

    const JNINativeMethod javaMethods[] {
        {"nativeInit", "()Z", reinterpret_cast<void *>(jniInit)}
    };

    QJniEnvironment jniEnv;
    (void) jniEnv.checkAndClearExceptions();

    jclass objectClass = jniEnv->FindClass(AndroidInterface::kJniQGCActivityClassName);
    if (!objectClass) {
        qCWarning(AndroidInitLog) << "Couldn't find class:" << AndroidInterface::kJniQGCActivityClassName;
        (void) jniEnv.checkAndClearExceptions();
        return JNI_ERR;
    }

    const jint val = jniEnv->RegisterNatives(objectClass, javaMethods, std::size(javaMethods));
    if (val < 0) {
        qCWarning(AndroidInitLog) << "Error registering methods:" << val;
        (void) jniEnv.checkAndClearExceptions();
        return JNI_ERR;
    }

    qCDebug(AndroidInitLog) << "Main Native Functions Registered";

    (void) jniEnv.checkAndClearExceptions();

    return JNI_OK;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    Q_UNUSED(reserved);

    qCDebug(AndroidInitLog) << Q_FUNC_INFO;

    // Store JavaVM for GStreamer helper function
    _java_vm = vm;

    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    if (jniSetNativeMethods() != JNI_OK) {
        return JNI_ERR;
    }

    #ifdef QGC_GST_STREAMING
        gst_amc_jni_set_java_vm(vm);
    #endif

    AndroidInterface::setNativeMethods();

    #ifndef QGC_NO_SERIAL_LINK
        AndroidSerial::setNativeMethods();
    #endif

    QNativeInterface::QAndroidApplication::hideSplashScreen(333);

    return JNI_VERSION_1_6;
}
