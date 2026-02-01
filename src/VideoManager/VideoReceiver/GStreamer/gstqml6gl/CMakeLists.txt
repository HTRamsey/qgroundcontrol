# ============================================================================
# GStreamer Qt6 QML GL Plugin
# Qt6 integration for GStreamer video rendering
# ============================================================================

qt_add_library(gstqml6gl STATIC)

target_link_libraries(gstqml6gl PUBLIC GStreamer::GStreamer)

# TODO: Skip custom build if system gstreamer1.0-qt6 plugin is found
# if(GST_PLUGIN_qml6_FOUND)
#     return()
# endif()

# ============================================================================
# GStreamer Good Plugins Integration
# ============================================================================

message(STATUS "gstqml6gl: GStreamer_VERSION = '${GStreamer_VERSION}'")

if(GStreamer_VERSION VERSION_GREATER_EQUAL "1.22")
    # Get recommended stable version for this GStreamer release
    string(REPLACE "." ";" GST_VERSION_LIST "${GStreamer_VERSION}")
    list(GET GST_VERSION_LIST 0 GST_VERSION_MAJOR)
    list(GET GST_VERSION_LIST 1 GST_VERSION_MINOR)

    # Use centralized version mapping from GStreamerConfig.cmake
    gstreamer_get_recommended_version(${GST_VERSION_MAJOR} ${GST_VERSION_MINOR} GST_PLUGINS_VERSION)

    # Try gst-plugins-good tarball first
    # https://gitlab.freedesktop.org/gstreamer/gstreamer/-/tree/${GST_PLUGINS_VERSION}/subprojects/gst-plugins-good/ext/qt6
    gstreamer_get_package_url(good_plugins ${GST_PLUGINS_VERSION} _gst_good_plugins_url)

    CPMAddPackage(
        NAME gstreamer_good_plugins
        VERSION ${GST_PLUGINS_VERSION}
        URL ${_gst_good_plugins_url}
    )

    if(gstreamer_good_plugins_SOURCE_DIR AND EXISTS "${gstreamer_good_plugins_SOURCE_DIR}/ext/qt6")
        set(QGC_GST_QT6_PLUGIN_PATH "${gstreamer_good_plugins_SOURCE_DIR}/ext/qt6")
        message(STATUS "gstqml6gl: Using downloaded gst-plugins-good Qt6 sources")
    else()
        message(STATUS "gstqml6gl: gst-plugins-good SOURCE_DIR='${gstreamer_good_plugins_SOURCE_DIR}'")
        # Fallback: Try GStreamer monorepo from GitLab
        message(STATUS "gst-plugins-good not available, trying GStreamer monorepo...")
        gstreamer_get_package_url(monorepo ${GST_PLUGINS_VERSION} _gst_monorepo_url)

        CPMAddPackage(
            NAME gstreamer_monorepo
            VERSION ${GST_PLUGINS_VERSION}
            URL ${_gst_monorepo_url}
        )

        if(gstreamer_monorepo_SOURCE_DIR AND EXISTS "${gstreamer_monorepo_SOURCE_DIR}/subprojects/gst-plugins-good/ext/qt6")
            set(QGC_GST_QT6_PLUGIN_PATH "${gstreamer_monorepo_SOURCE_DIR}/subprojects/gst-plugins-good/ext/qt6")
        else()
            message(FATAL_ERROR "Failed to download GStreamer Qt6 plugins from both sources")
        endif()
    endif()
else()
    message(STATUS "gstqml6gl: GStreamer < 1.22, using local Qt6 fallback sources")
    set(QGC_GST_QT6_PLUGIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/qt6")
endif()

message(STATUS "gstqml6gl: QGC_GST_QT6_PLUGIN_PATH = ${QGC_GST_QT6_PLUGIN_PATH}")

# ============================================================================
# Source Code Patching
# ============================================================================

# Fix missing includes in GStreamer Qt6 plugin
file(READ "${QGC_GST_QT6_PLUGIN_PATH}/qt6glitem.h" FILE_CONTENTS)
string(FIND "${FILE_CONTENTS}" "#include <QtQuick/QQuickWindow>" GST_FIX_INCLUDES)
if(GST_FIX_INCLUDES EQUAL -1)
    string(REPLACE "#include <QtGui/QOpenGLFunctions>" "#include <QtGui/QOpenGLFunctions>\n#include <QtQuick/QQuickWindow>" FILE_CONTENTS "${FILE_CONTENTS}")
    file(WRITE "${QGC_GST_QT6_PLUGIN_PATH}/qt6glitem.h" "${FILE_CONTENTS}")
endif()

# ----------------------------------------------------------------------------
# Build GStreamer QML6 GL Library
# ----------------------------------------------------------------------------

# NOTE: Using file(GLOB) for external GStreamer plugin sources
# CONFIGURE_DEPENDS ensures CMake re-runs if files are added/removed
file(GLOB gstqml6gl_SRCS
    CONFIGURE_DEPENDS
    "${QGC_GST_QT6_PLUGIN_PATH}/*.cc"
    "${QGC_GST_QT6_PLUGIN_PATH}/*.h"
)
target_sources(gstqml6gl PRIVATE ${gstqml6gl_SRCS})

target_link_libraries(gstqml6gl
    PRIVATE
        Qt6::GuiPrivate
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
)

target_include_directories(gstqml6gl PUBLIC ${QGC_GST_QT6_PLUGIN_PATH})

target_precompile_headers(gstqml6gl
    PRIVATE
        <gst/gst.h>
        <gst/gl/gl.h>
        <gst/video/video.h>
)

# ============================================================================
# Shader Compilation
# ============================================================================

if(EXISTS "${QGC_GST_QT6_PLUGIN_PATH}/resources.qrc")
    find_package(Qt6 REQUIRED COMPONENTS ShaderTools)

    set(SHADERS
        "${QGC_GST_QT6_PLUGIN_PATH}/vertex.vert"
        "${QGC_GST_QT6_PLUGIN_PATH}/YUV_TRIPLANAR.frag"
        "${QGC_GST_QT6_PLUGIN_PATH}/RGBA.frag"
    )

    set(OUTPUTS
        vertex.vert.qsb
        YUV_TRIPLANAR.frag.qsb
        RGBA.frag.qsb
    )

    if(EXISTS "${QGC_GST_QT6_PLUGIN_PATH}/YUV_BIPLANAR.frag")
        list(APPEND SHADERS "${QGC_GST_QT6_PLUGIN_PATH}/YUV_BIPLANAR.frag")
        list(APPEND OUTPUTS "YUV_BIPLANAR.frag.qsb")
    endif()

    # Compile shaders for different GLSL versions
    qt_add_shaders(gstqml6gl "gstqml6gl_shaders"
        PREFIX "/org/freedesktop/gstreamer/qml6"
        GLSL "100 es,120,330"
        OUTPUT_TARGETS gstqml6gl_shaders
        FILES ${SHADERS}
        OUTPUTS ${OUTPUTS}
        BATCHABLE
    )

    # Handle additional GLES-specific RGBA shader if present
    if(EXISTS "${QGC_GST_QT6_PLUGIN_PATH}/RGBA_gles.frag")
        find_program(QSB_PROGRAM
            NAMES qsb
            HINTS ${QT_HOST_PATH} ${QT_ROOT_DIR} ${QTDIR}
            ENV QTDIR
            PATH_SUFFIXES bin
            REQUIRED
        )

        find_program(QSB_WRAPPER
            NAMES qsb-wrapper.py
            PATHS "${QGC_GST_QT6_PLUGIN_PATH}"
            REQUIRED
        )

        set(RGBA_BASE_QSB "${CMAKE_CURRENT_BINARY_DIR}/.qsb/RGBA.frag.qsb")
        set(RGBA_GLES_SRC "${QGC_GST_QT6_PLUGIN_PATH}/RGBA_gles.frag")
        set(RGBA_EXTERNAL "${CMAKE_CURRENT_BINARY_DIR}/.qsb/RGBA.frag.qsb.external")

        add_custom_command(
            OUTPUT ${RGBA_EXTERNAL}
            COMMAND ${QSB_WRAPPER}
                ${QSB_PROGRAM}
                ${RGBA_EXTERNAL}
                ${RGBA_GLES_SRC}
                ${RGBA_BASE_QSB}
            DEPENDS
                ${RGBA_BASE_QSB}
                ${RGBA_GLES_SRC}
            COMMENT "Generating external GLES shader: ${RGBA_EXTERNAL}"
            VERBATIM
        )

        set_source_files_properties("${RGBA_EXTERNAL}" PROPERTIES QT_RESOURCE_ALIAS RGBA.frag.qsb.external)
        qt_add_resources(gstqml6gl "gstqml6gl_shaders1"
            PREFIX "/org/freedesktop/gstreamer/qml6"
            FILES "${RGBA_EXTERNAL}"
        )
    endif()
endif()

# ============================================================================
# Platform-Specific OpenGL Support
# ============================================================================

if(GStreamer_GlX11_FOUND)
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_X11)
endif()

if(GStreamer_GlEgl_FOUND)
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_EGLFS)
endif()

if(GStreamer_GlWayland_FOUND)
    find_package(Qt6 COMPONENTS WaylandClient)
    if(TARGET Qt6::WaylandClient)
        target_link_libraries(gstqml6gl PRIVATE Qt6::WaylandClient)
    endif()
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_WAYLAND)
endif()

if(ANDROID)
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_ANDROID)
elseif(WIN32)
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_WIN32)
elseif(MACOS)
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_MAC)
elseif(IOS)
    target_compile_definitions(gstqml6gl PRIVATE HAVE_QT_IOS)
endif()

target_compile_definitions(gstqml6gl
    PRIVATE
        HAVE_QT_QPA_HEADER
        QT_QPA_HEADER=<QtGui/qpa/qplatformnativeinterface.h>
)
