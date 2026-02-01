# ============================================================================
# FindGStreamerQGC.cmake
# QGC find module for GStreamer multimedia framework
#
# Handles platform detection and GStreamer download/setup, then delegates to
# official GStreamer CMake modules for target creation.
#
# Platforms: Windows, Linux, macOS, Android, iOS
# ============================================================================

# Load shared GStreamer configuration (URLs, exclusion lists, install functions)
include(GStreamerConfig)

# ============================================================================
# Default Configuration
# ============================================================================

# Set default version based on platform (from build-config.json)
if(NOT DEFINED GStreamer_FIND_VERSION)
    if(LINUX)
        set(GStreamer_FIND_VERSION 1.20)
    elseif(WIN32)
        set(GStreamer_FIND_VERSION ${QGC_CONFIG_GSTREAMER_WIN_VERSION})
    elseif(ANDROID)
        set(GStreamer_FIND_VERSION ${QGC_CONFIG_GSTREAMER_ANDROID_VERSION})
    else()
        set(GStreamer_FIND_VERSION ${QGC_CONFIG_GSTREAMER_VERSION})
    endif()
endif()

# Determine GStreamer root directory from various environment variables
if(NOT DEFINED GStreamer_ROOT_DIR)
    if(DEFINED GSTREAMER_ROOT)
        set(GStreamer_ROOT_DIR ${GSTREAMER_ROOT})
    elseif(DEFINED GStreamer_ROOT)
        set(GStreamer_ROOT_DIR ${GStreamer_ROOT})
    endif()

    if(DEFINED GStreamer_ROOT_DIR AND NOT EXISTS "${GStreamer_ROOT_DIR}")
        message(STATUS "GStreamer: User-provided directory does not exist: ${GStreamer_ROOT_DIR}")
    endif()
endif()

# Static vs dynamic linking preference
if(NOT DEFINED GStreamer_USE_STATIC_LIBS)
    if(ANDROID OR IOS)
        set(GStreamer_USE_STATIC_LIBS ON)
    else()
        set(GStreamer_USE_STATIC_LIBS OFF)
    endif()
endif()

# Framework usage (macOS/iOS only)
if(NOT DEFINED GStreamer_USE_FRAMEWORK)
    if(APPLE)
        set(GStreamer_USE_FRAMEWORK ON)
    else()
        set(GStreamer_USE_FRAMEWORK OFF)
    endif()
endif()

# ============================================================================
# Platform-Specific Path Setup
# ============================================================================

set(PKG_CONFIG_ARGN)

# ----------------------------------------------------------------------------
# Windows Platform
# ----------------------------------------------------------------------------
if(WIN32)
    if(NOT DEFINED GStreamer_ROOT_DIR)
        if(DEFINED ENV{GSTREAMER_1_0_ROOT_X86_64} AND EXISTS "$ENV{GSTREAMER_1_0_ROOT_X86_64}")
            set(GStreamer_ROOT_DIR "$ENV{GSTREAMER_1_0_ROOT_X86_64}")
        elseif(MSVC AND DEFINED ENV{GSTREAMER_1_0_ROOT_MSVC_X86_64} AND EXISTS "$ENV{GSTREAMER_1_0_ROOT_MSVC_X86_64}")
            set(GStreamer_ROOT_DIR "$ENV{GSTREAMER_1_0_ROOT_MSVC_X86_64}")
        elseif(MINGW AND DEFINED ENV{GSTREAMER_1_0_ROOT_MINGW_X86_64} AND EXISTS "$ENV{GSTREAMER_1_0_ROOT_MINGW_X86_64}")
            set(GStreamer_ROOT_DIR "$ENV{GSTREAMER_1_0_ROOT_MINGW_X86_64}")
        elseif(EXISTS "C:/Program Files/gstreamer/1.0/msvc_x86_64")
            set(GStreamer_ROOT_DIR "C:/Program Files/gstreamer/1.0/msvc_x86_64")
        elseif(EXISTS "C:/gstreamer/1.0/msvc_x86_64")
            set(GStreamer_ROOT_DIR "C:/gstreamer/1.0/msvc_x86_64")
        endif()
    endif()

    cmake_path(CONVERT "${GStreamer_ROOT_DIR}" TO_CMAKE_PATH_LIST GStreamer_ROOT_DIR NORMALIZE)
    if(NOT EXISTS "${GStreamer_ROOT_DIR}")
        message(FATAL_ERROR "Could not locate GStreamer - check installation or set environment/cmake variables")
    endif()

    set(GSTREAMER_LIB_PATH "${GStreamer_ROOT_DIR}/lib")
    set(GSTREAMER_PLUGIN_PATH "${GSTREAMER_LIB_PATH}/gstreamer-1.0")
    set(GSTREAMER_INCLUDE_PATH "${GStreamer_ROOT_DIR}/include")

    set(ENV{PKG_CONFIG} "${GStreamer_ROOT_DIR}/bin")
    set(PKG_CONFIG_EXECUTABLE "$ENV{PKG_CONFIG}/pkg-config.exe")
    set(ENV{PKG_CONFIG_PATH} "${GSTREAMER_LIB_PATH}/pkgconfig;${GSTREAMER_PLUGIN_PATH}/pkgconfig;$ENV{PKG_CONFIG_PATH}")
    list(APPEND PKG_CONFIG_ARGN
        --dont-define-prefix
        --define-variable=prefix=${GStreamer_ROOT_DIR}
        --define-variable=libdir=${GSTREAMER_LIB_PATH}
        --define-variable=includedir=${GSTREAMER_INCLUDE_PATH}
    )

# ----------------------------------------------------------------------------
# Linux Platform
# ----------------------------------------------------------------------------
elseif(LINUX)
    if(NOT DEFINED GStreamer_ROOT_DIR)
        if(EXISTS "/usr")
            set(GStreamer_ROOT_DIR "/usr")
        endif()
    endif()

    cmake_path(CONVERT "${GStreamer_ROOT_DIR}" TO_CMAKE_PATH_LIST GStreamer_ROOT_DIR NORMALIZE)
    if(NOT EXISTS "${GStreamer_ROOT_DIR}")
        message(FATAL_ERROR "Could not locate GStreamer - check installation or set environment/cmake variables")
    endif()

    # Try multiarch path first (Debian/Ubuntu), then lib64 (Fedora/RHEL), then lib (Arch)
    if((EXISTS "${GStreamer_ROOT_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu") AND (EXISTS "${GStreamer_ROOT_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/gstreamer-1.0"))
        set(GSTREAMER_LIB_PATH "${GStreamer_ROOT_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu")
    elseif((EXISTS "${GStreamer_ROOT_DIR}/lib64") AND (EXISTS "${GStreamer_ROOT_DIR}/lib64/gstreamer-1.0"))
        set(GSTREAMER_LIB_PATH "${GStreamer_ROOT_DIR}/lib64")
    elseif((EXISTS "${GStreamer_ROOT_DIR}/lib") AND (EXISTS "${GStreamer_ROOT_DIR}/lib/gstreamer-1.0"))
        set(GSTREAMER_LIB_PATH "${GStreamer_ROOT_DIR}/lib")
    else()
        message(FATAL_ERROR "Could not locate GStreamer libraries - check installation or set environment/cmake variables")
    endif()

    set(GSTREAMER_PLUGIN_PATH "${GSTREAMER_LIB_PATH}/gstreamer-1.0")
    set(GSTREAMER_INCLUDE_PATH "${GStreamer_ROOT_DIR}/include")

    set(ENV{PKG_CONFIG_PATH} "${GSTREAMER_LIB_PATH}/pkgconfig:$ENV{PKG_CONFIG_PATH}")

# ----------------------------------------------------------------------------
# Android Platform
# ----------------------------------------------------------------------------
elseif(ANDROID)
    gstreamer_get_package_url(android ${GStreamer_FIND_VERSION} _gst_android_url)

    CPMAddPackage(
        NAME gstreamer
        VERSION ${GStreamer_FIND_VERSION}
        URL ${_gst_android_url}
    )

    if(NOT DEFINED GStreamer_ROOT_DIR)
        if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
            set(GStreamer_ROOT_DIR "${gstreamer_SOURCE_DIR}/armv7")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
            set(GStreamer_ROOT_DIR "${gstreamer_SOURCE_DIR}/arm64")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
            set(GStreamer_ROOT_DIR "${gstreamer_SOURCE_DIR}/x86")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
            set(GStreamer_ROOT_DIR "${gstreamer_SOURCE_DIR}/x86_64")
        endif()
    endif()

    cmake_path(CONVERT "${GStreamer_ROOT_DIR}" TO_CMAKE_PATH_LIST GStreamer_ROOT_DIR NORMALIZE)
    if(NOT EXISTS "${GStreamer_ROOT_DIR}")
        message(FATAL_ERROR "Could not locate GStreamer - check installation or set environment/cmake variables")
    endif()

    set(GSTREAMER_LIB_PATH "${GStreamer_ROOT_DIR}/lib")
    set(GSTREAMER_PLUGIN_PATH "${GSTREAMER_LIB_PATH}/gstreamer-1.0")
    set(GSTREAMER_INCLUDE_PATH "${GStreamer_ROOT_DIR}/include")

    set(ENV{PKG_CONFIG_PATH} "")
    if(CMAKE_HOST_WIN32)
        set(ENV{PKG_CONFIG} "${GStreamer_ROOT_DIR}/share/gst-android/ndk-build/tools/windows")
        set(PKG_CONFIG_EXECUTABLE "$ENV{PKG_CONFIG}/pkg-config.exe")
        set(ENV{PKG_CONFIG_LIBDIR} "${GSTREAMER_LIB_PATH}/pkgconfig;${GSTREAMER_PLUGIN_PATH}/pkgconfig")
        list(APPEND PKG_CONFIG_ARGN --dont-define-prefix)
    elseif(CMAKE_HOST_UNIX)
        if(CMAKE_HOST_APPLE)
            find_program(PKG_CONFIG_EXECUTABLE
                NAMES pkg-config
                PATHS /opt/homebrew/bin /usr/local/bin
                NO_DEFAULT_PATH
            )
            if(NOT PKG_CONFIG_EXECUTABLE)
                find_program(PKG_CONFIG_EXECUTABLE pkg-config)
            endif()
            if(NOT PKG_CONFIG_EXECUTABLE)
                message(FATAL_ERROR "Could not find pkg-config. Please install pkg-config using tools/setup/install-dependencies-osx.sh.")
            endif()
        endif()
        set(ENV{PKG_CONFIG_LIBDIR} "${GSTREAMER_LIB_PATH}/pkgconfig:${GSTREAMER_PLUGIN_PATH}/pkgconfig")
    endif()
    list(APPEND PKG_CONFIG_ARGN
        --define-variable=prefix=${GStreamer_ROOT_DIR}
        --define-variable=libdir=${GSTREAMER_LIB_PATH}
        --define-variable=includedir=${GSTREAMER_INCLUDE_PATH}
    )

# ----------------------------------------------------------------------------
# macOS Platform
# ----------------------------------------------------------------------------
elseif(MACOS)
    if(NOT DEFINED GStreamer_ROOT_DIR)
        if(EXISTS "/Library/Frameworks/GStreamer.framework")
            set(GStreamer_ROOT_DIR "/Library/Frameworks/GStreamer.framework/Versions/1.0")
        elseif(EXISTS "/opt/homebrew/opt/gstreamer")
            set(GStreamer_ROOT_DIR "/opt/homebrew/opt/gstreamer")
        elseif(EXISTS "/usr/local/opt/gstreamer")
            set(GStreamer_ROOT_DIR "/usr/local/opt/gstreamer")
        endif()
    endif()

    cmake_path(CONVERT "${GStreamer_ROOT_DIR}" TO_CMAKE_PATH_LIST GStreamer_ROOT_DIR NORMALIZE)
    if(NOT EXISTS "${GStreamer_ROOT_DIR}")
        message(FATAL_ERROR "Could not locate GStreamer - check installation or set environment/cmake variables")
    endif()

    if(GStreamer_USE_FRAMEWORK)
        set(GSTREAMER_FRAMEWORK_PATH "${GStreamer_ROOT_DIR}/../..")
        cmake_path(NORMAL_PATH GSTREAMER_FRAMEWORK_PATH)
        set(GSTREAMER_FRAMEWORK "${GSTREAMER_FRAMEWORK_PATH}")
    endif()

    set(GSTREAMER_INCLUDE_PATH "${GStreamer_ROOT_DIR}/include")
    set(GSTREAMER_LIB_PATH "${GStreamer_ROOT_DIR}/lib")
    set(GSTREAMER_PLUGIN_PATH "${GSTREAMER_LIB_PATH}/gstreamer-1.0")

    set(ENV{PKG_CONFIG} "${GStreamer_ROOT_DIR}/bin")
    set(PKG_CONFIG_EXECUTABLE "$ENV{PKG_CONFIG}/pkg-config")
    set(ENV{PKG_CONFIG_PATH} "${GSTREAMER_LIB_PATH}/pkgconfig:${GSTREAMER_PLUGIN_PATH}/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    list(APPEND PKG_CONFIG_ARGN
        --dont-define-prefix
        --define-variable=prefix=${GStreamer_ROOT_DIR}
        --define-variable=libdir=${GSTREAMER_LIB_PATH}
        --define-variable=includedir=${GSTREAMER_INCLUDE_PATH}
    )

# ----------------------------------------------------------------------------
# iOS Platform
# ----------------------------------------------------------------------------
elseif(IOS)
    if(NOT CMAKE_HOST_APPLE)
        message(FATAL_ERROR "GStreamer for iOS can only be built on macOS")
    endif()

    # Check for pre-installed GStreamer iOS SDK
    if(NOT DEFINED GStreamer_ROOT_DIR)
        if(EXISTS "/Library/Developer/GStreamer/iPhone.sdk/GStreamer.framework")
            set(GSTREAMER_FRAMEWORK_PATH "/Library/Developer/GStreamer/iPhone.sdk/GStreamer.framework")
            set(GStreamer_ROOT_DIR "${GSTREAMER_FRAMEWORK_PATH}/Versions/1.0")
        endif()
    endif()

    # Download and extract if not found
    if(NOT EXISTS "${GStreamer_ROOT_DIR}")
        gstreamer_get_package_url(ios ${GStreamer_FIND_VERSION} _gst_ios_url)

        set(_gst_ios_cache_dir "${CMAKE_BINARY_DIR}/_deps/gstreamer-ios")
        set(_gst_ios_pkg "${_gst_ios_cache_dir}/gstreamer-ios.pkg")
        set(_gst_ios_expanded "${_gst_ios_cache_dir}/expanded")

        if(NOT EXISTS "${_gst_ios_pkg}")
            message(STATUS "Downloading GStreamer iOS SDK...")
            file(MAKE_DIRECTORY "${_gst_ios_cache_dir}")
            file(DOWNLOAD
                "${_gst_ios_url}"
                "${_gst_ios_pkg}"
                SHOW_PROGRESS
                STATUS _download_status
            )
            list(GET _download_status 0 _download_rc)
            if(NOT _download_rc EQUAL 0)
                list(GET _download_status 1 _download_error)
                message(FATAL_ERROR "Failed to download GStreamer iOS SDK: ${_download_error}")
            endif()
        endif()

        if(NOT EXISTS "${_gst_ios_expanded}")
            message(STATUS "Expanding GStreamer iOS package...")
            file(MAKE_DIRECTORY "${_gst_ios_expanded}")
            execute_process(
                COMMAND pkgutil --expand-full "${_gst_ios_pkg}" "${_gst_ios_expanded}"
                RESULT_VARIABLE _pkgutil_rc
            )
            if(NOT _pkgutil_rc EQUAL 0)
                message(FATAL_ERROR "pkgutil failed to expand GStreamer .pkg")
            endif()
        endif()

        file(GLOB _pkg_dirs "${_gst_ios_expanded}/*.pkg")
        foreach(_pkg_dir IN LISTS _pkg_dirs)
            if(EXISTS "${_pkg_dir}/Payload/Library/Developer/GStreamer/iPhone.sdk/GStreamer.framework")
                set(GSTREAMER_FRAMEWORK_PATH "${_pkg_dir}/Payload/Library/Developer/GStreamer/iPhone.sdk/GStreamer.framework")
                break()
            endif()
        endforeach()

        if(NOT GSTREAMER_FRAMEWORK_PATH)
            message(FATAL_ERROR "Could not locate GStreamer.framework in downloaded iOS SDK")
        endif()

        set(GStreamer_ROOT_DIR "${GSTREAMER_FRAMEWORK_PATH}/Versions/1.0")
    endif()

    cmake_path(CONVERT "${GStreamer_ROOT_DIR}" TO_CMAKE_PATH_LIST GStreamer_ROOT_DIR NORMALIZE)
    if(NOT EXISTS "${GStreamer_ROOT_DIR}")
        message(FATAL_ERROR "Could not locate GStreamer iOS SDK - install from https://gstreamer.freedesktop.org/download/ or set GStreamer_ROOT_DIR")
    endif()

    set(GStreamer_USE_FRAMEWORK ON)
    set(GSTREAMER_INCLUDE_PATH "${GSTREAMER_FRAMEWORK_PATH}/Headers")
    set(GSTREAMER_LIB_PATH "${GStreamer_ROOT_DIR}/lib")
    set(GSTREAMER_PLUGIN_PATH "${GSTREAMER_LIB_PATH}/gstreamer-1.0")

    set(ENV{PKG_CONFIG_PATH} "${GSTREAMER_LIB_PATH}/pkgconfig:${GSTREAMER_PLUGIN_PATH}/pkgconfig")
endif()

# ============================================================================
# Validation
# ============================================================================
if(NOT EXISTS "${GStreamer_ROOT_DIR}" OR NOT EXISTS "${GSTREAMER_LIB_PATH}" OR NOT EXISTS "${GSTREAMER_PLUGIN_PATH}" OR NOT EXISTS "${GSTREAMER_INCLUDE_PATH}")
    message(FATAL_ERROR "GStreamer: Could not locate required directories - check installation or set GStreamer_ROOT_DIR")
endif()

if(GStreamer_USE_FRAMEWORK AND NOT EXISTS "${GSTREAMER_FRAMEWORK_PATH}")
    message(FATAL_ERROR "GStreamer: Could not locate framework at ${GSTREAMER_FRAMEWORK_PATH}")
endif()

# ============================================================================
# Plugin & Dependency Configuration
# ============================================================================

# Extra dependencies for pkg-config (used by _FindGStreamer.cmake)
set(GSTREAMER_EXTRA_DEPS
    gstreamer-base-1.0
    gstreamer-video-1.0
    gstreamer-gl-1.0
    gstreamer-gl-prototypes-1.0
    gstreamer-rtsp-1.0
)

# Plugins for static linking (cross-platform base set)
set(GSTREAMER_PLUGINS
    coreelements
    isomp4
    libav
    matroska
    mpegtsdemux
    opengl
    openh264
    playback
    rtp
    rtpmanager
    rtsp
    sdpelem
    tcp
    typefindfunctions
    udp
    videoparsersbad
    vpx
)

# Platform-specific plugins
if(ANDROID)
    list(APPEND GSTREAMER_PLUGINS androidmedia)
elseif(APPLE)
    # dav1d for AV1 software decoding, applemedia for hardware
    list(APPEND GSTREAMER_PLUGINS applemedia dav1d vulkan)
elseif(WIN32)
    list(APPEND GSTREAMER_PLUGINS d3d d3d11 d3d12 dav1d dxva nvcodec)
elseif(LINUX)
    list(APPEND GSTREAMER_PLUGINS dav1d nvcodec qsv va vulkan)
endif()

# API packages to find
set(GSTREAMER_APIS
    api_video
    api_gl
    api_rtsp
)

# ============================================================================
# Mobile Configuration (Android/iOS)
# ============================================================================
if(ANDROID)
    set(GStreamer_Mobile_MODULE_NAME gstreamer_android)
    set(G_IO_MODULES openssl)
    set(G_IO_MODULES_PATH "${GStreamer_ROOT_DIR}/lib/gio/modules")
    set(GStreamer_NDK_BUILD_PATH "${GStreamer_ROOT_DIR}/share/gst-android/ndk-build/")
    set(GStreamer_JAVA_SRC_DIR "${CMAKE_BINARY_DIR}/android-build-${CMAKE_PROJECT_NAME}/src")
    set(GStreamer_ASSETS_DIR "${CMAKE_BINARY_DIR}/android-build-${CMAKE_PROJECT_NAME}/assets")
elseif(IOS)
    set(GStreamer_Mobile_MODULE_NAME gstreamer_mobile)
    set(G_IO_MODULES openssl)
    set(G_IO_MODULES_PATH "${GStreamer_ROOT_DIR}/lib/gio/modules")
    set(GStreamer_ASSETS_DIR "${CMAKE_BINARY_DIR}/assets")
endif()

# ============================================================================
# Delegate to Official GStreamer CMake Modules
# ============================================================================

# Use official GStreamer find module for target creation
include(FindGStreamer)

# For mobile platforms, also use the mobile module
if(ANDROID OR IOS)
    set(GStreamerMobile_FIND_COMPONENTS ${GSTREAMER_PLUGINS} mobile ca_certificates)
    if(GStreamer_FIND_REQUIRED)
        set(GStreamerMobile_FIND_REQUIRED TRUE)
    endif()
    # QGC provides its own JNI_OnLoad in AndroidInit.cc, so skip the JNI wrapper
    # from the GStreamer template to avoid duplicate symbol errors
    set(GStreamer_Mobile_NO_JNI_WRAPPER TRUE)
    include(FindGStreamerMobile)
endif()

# ============================================================================
# Component Compatibility Layer
# ============================================================================
# Set component _FOUND variables for compatibility with existing code
# that uses find_package(GStreamer COMPONENTS ...)

find_package(PkgConfig REQUIRED QUIET)

# Core components (always required)
foreach(_comp IN ITEMS Core Base Video Gl GlPrototypes Rtsp)
    set(GStreamer_${_comp}_FOUND ${GStreamer_FOUND})
endforeach()

# Optional GL platform components
pkg_check_modules(_GST_GL_EGL QUIET gstreamer-gl-egl-1.0)
if(_GST_GL_EGL_FOUND)
    set(GStreamer_GlEgl_FOUND TRUE)
endif()

pkg_check_modules(_GST_GL_WAYLAND QUIET gstreamer-gl-wayland-1.0)
if(_GST_GL_WAYLAND_FOUND)
    set(GStreamer_GlWayland_FOUND TRUE)
endif()

pkg_check_modules(_GST_GL_X11 QUIET gstreamer-gl-x11-1.0)
if(_GST_GL_X11_FOUND)
    set(GStreamer_GlX11_FOUND TRUE)
endif()

# ============================================================================
# macOS Framework Handling
# ============================================================================
if(MACOS AND GStreamer_USE_FRAMEWORK AND TARGET GStreamer::GStreamer)
    set(CMAKE_FIND_FRAMEWORK ONLY)
    find_library(GSTREAMER_FRAMEWORK GStreamer
        PATHS
            "${GSTREAMER_FRAMEWORK_PATH}"
            "/Library/Frameworks"
            "/usr/local/opt/gstreamer"
            "/opt/homebrew/opt/gstreamer"
    )
    unset(CMAKE_FIND_FRAMEWORK)

    if(GSTREAMER_FRAMEWORK)
        target_compile_definitions(GStreamer::GStreamer INTERFACE QGC_GST_MACOS_FRAMEWORK)
    endif()
endif()
