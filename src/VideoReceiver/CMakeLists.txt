#######################################################
#                GStreamer Configuration
#######################################################

option(ENABLE_VIDEOSTREAMING "Enable video streaming" ON)
if (ENABLE_VIDEOSTREAMING AND NOT MOBILE)
    message(STATUS "Enabling video streaming support")
    find_package(PkgConfig)

    set(GST_DEPENDENCIES
            gstreamer-1.0>=1.20
            gstreamer-video-1.0>=1.20
            gstreamer-gl-1.0>=1.20
    )

    if (LINUX OR ANDROID)
        set(GST_DEPENDENCIES ${GST_DEPENDENCIES} egl)
    endif ()

    pkg_check_modules(GST
            ${GST_DEPENDENCIES}
    )

    message(STATUS "GStreamer libs: ${GST_LIBRARIES}")
    message(STATUS "GStreamer include dirs: ${GST_INCLUDE_DIRS}")
    message(STATUS "GStreamer link dirs: ${GST_LIBRARY_DIRS}")
    message(STATUS "GStreamer cflags: ${GST_CFLAGS}")
    message(STATUS "GStreamer ldflags: ${GST_LDFLAGS}")
    message(STATUS "GStreamer libs: ${GST_LIBS}")

    if (LINUX OR ANDROID)
        message(STATUS "GStreamer egl libs: ${GST_EGL_LIBRARIES}")
        message(STATUS "GStreamer egl include dirs: ${GST_EGL_INCLUDE_DIRS}")
        message(STATUS "GStreamer egl link dirs: ${GST_EGL_LIBRARY_DIRS}")
        message(STATUS "GStreamer egl cflags: ${GST_EGL_CFLAGS}")
        message(STATUS "GStreamer egl ldflags: ${GST_EGL_LDFLAGS}")
        message(STATUS "GStreamer egl libs: ${GST_EGL_LIBS}")
    endif ()

    message(STATUS "gst found ${GST_FOUND}")
    if (GST_FOUND)
        add_definitions(-DQGC_GST_STREAMING)
        option(QGC_GST_MICROHARD_ENABLED "Enable microhard" OFF)
        option(QGC_GST_TAISYNC_ENABLED "Enable taisyng" OFF)
    else ()
        message(STATUS "Tried to enable video streaming support, but GStreamer libs not found")
        if (QGC_GST_MICROHARD_ENABLED OR QGC_GST_TAISYNC_ENABLED)
            message(FATAL_ERROR "You tried to enable Microhard or Taisync but gstreamer is not found. Make sure to set PKG_CONFIG_EXECUTABLE and/or PKG_CONFIG_PATH properly.")
        endif ()
    endif ()
endif ()

set(EXTRA_SOURCES)
set(EXTRA_LIBRARIES)

if (GST_FOUND)
    set(EXTRA_SOURCES
    	gstqgc.c
    	gstqgcvideosinkbin.c
    	GStreamer.cc
    	GStreamer.h
    	GstVideoReceiver.cc
    	GstVideoReceiver.h
    )

    set(EXTRA_LIBRARIES qmlglsink ${GST_LINK_LIBRARIES})
endif()

qt_add_library(VideoReceiver
    STATIC
    ${EXTRA_SOURCES}
    VideoReceiver.h
)

target_link_libraries(VideoReceiver
    PUBLIC
        Qt6::Multimedia
        Qt6::OpenGL
        Qt6::Quick
        ${EXTRA_LIBRARIES}
)

target_include_directories(VideoReceiver INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
if (GST_FOUND)
	target_include_directories(VideoReceiver PUBLIC ${GST_INCLUDE_DIRS})
endif()
