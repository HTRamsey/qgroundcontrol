# ============================================================================
# GStreamer Video Receiver Backend
# GStreamer-based video streaming and decoding
# ============================================================================

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE VideoItemStub.h)

# ============================================================================
# GStreamer Detection and Configuration
# ============================================================================

if(QGC_ENABLE_GST_VIDEOSTREAMING)
    find_package(GStreamerQGC REQUIRED)

    # Build GStreamer Qt6 QML GL plugin
    add_subdirectory(gstqml6gl)

    # TODO: Add Qt6 Direct3D11 plugin support
    # https://gstreamer.freedesktop.org/documentation/qt6d3d11/index.html#qml6d3d11sink-page
endif()

# ============================================================================
# GStreamer Video Receiver Sources
# ============================================================================

if(TARGET gstqml6gl)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE gstqml6gl)

    target_sources(${CMAKE_PROJECT_NAME}
        PRIVATE
            GStreamer.cc
            GStreamer.h
            GStreamerHelpers.cc
            GStreamerHelpers.h
            GstVideoReceiver.cc
            GstVideoReceiver.h
    )

    # Build custom GStreamer QGC plugin
    add_subdirectory(gstqgc)

    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE QGC_GST_STREAMING)

    # ============================================================================
    # Platform-Specific GStreamer Installation
    # Note: Android/iOS use static linking via FindGStreamerMobile.cmake
    # ============================================================================

    if(LINUX)
        # ----------------------------------------------------------------------------
        # Linux: Install GStreamer Plugins and Libraries
        # ----------------------------------------------------------------------------
        gstreamer_install_plugins(
            SOURCE_DIR "${GSTREAMER_PLUGIN_PATH}"
            DEST_DIR "${CMAKE_INSTALL_LIBDIR}/gstreamer-1.0"
            EXTENSION "so"
            PREFIX "libgst"
        )
        gstreamer_install_gio_modules(
            SOURCE_DIR "${GSTREAMER_LIB_PATH}/gio/modules"
            DEST_DIR "${CMAKE_INSTALL_LIBDIR}/gio/modules"
            EXTENSION "so"
        )
        install(
            DIRECTORY "${GSTREAMER_LIB_PATH}/gstreamer1.0"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            FILE_PERMISSIONS
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ             GROUP_EXECUTE
                WORLD_READ             WORLD_EXECUTE
            FILES_MATCHING
                PATTERN "gst-*"
        )
    elseif(WIN32)
        # ----------------------------------------------------------------------------
        # Windows: Install GStreamer DLLs and Plugins
        # ----------------------------------------------------------------------------
        gstreamer_install_libs(
            SOURCE_DIR "${GStreamer_ROOT_DIR}/bin"
            DEST_DIR "${CMAKE_INSTALL_BINDIR}"
            EXTENSION "dll"
        )
        gstreamer_install_gio_modules(
            SOURCE_DIR "${GSTREAMER_LIB_PATH}/gio/modules"
            DEST_DIR "${CMAKE_INSTALL_LIBDIR}/gio/modules"
            EXTENSION "dll"
        )
        gstreamer_install_plugins(
            SOURCE_DIR "${GSTREAMER_PLUGIN_PATH}"
            DEST_DIR "${CMAKE_INSTALL_LIBDIR}/gstreamer-1.0"
            EXTENSION "dll"
            PREFIX "gst"
        )
        install(
            DIRECTORY "${GStreamer_ROOT_DIR}/libexec/gstreamer-1.0/"
            DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/gstreamer-1.0"
            FILE_PERMISSIONS
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ             GROUP_EXECUTE
                WORLD_READ             WORLD_EXECUTE
            FILES_MATCHING
                PATTERN "*.exe"
        )
    elseif(MACOS)
        # ----------------------------------------------------------------------------
        # macOS: Install GStreamer Framework with size optimization
        # ----------------------------------------------------------------------------
        if(GSTREAMER_FRAMEWORK)
            set(_macos_fw_dest "${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}.app/Contents/Frameworks/GStreamer.framework")

            # Install framework structure excluding plugins (will be filtered separately)
            install(
                DIRECTORY "${GSTREAMER_FRAMEWORK}"
                DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}.app/Contents/Frameworks"
                PATTERN "*.la" EXCLUDE
                PATTERN "*.a" EXCLUDE
                PATTERN "*/bin" EXCLUDE
                PATTERN "*/etc" EXCLUDE
                PATTERN "*/gst-validate-launcher" EXCLUDE
                PATTERN "*/Headers" EXCLUDE
                PATTERN "*/include" EXCLUDE
                PATTERN "*/pkgconfig" EXCLUDE
                PATTERN "*/share" EXCLUDE
                # Exclude plugins and GIO modules - will install filtered versions
                PATTERN "*/gstreamer-1.0" EXCLUDE
                PATTERN "*/gio/modules" EXCLUDE
            )

            # Install filtered plugins
            gstreamer_install_plugins(
                SOURCE_DIR "${GSTREAMER_PLUGIN_PATH}"
                DEST_DIR "${_macos_fw_dest}/Versions/1.0/lib/gstreamer-1.0"
                EXTENSION "dylib"
                PREFIX "libgst"
            )

            # Install filtered GIO modules
            gstreamer_install_gio_modules(
                SOURCE_DIR "${GSTREAMER_LIB_PATH}/gio/modules"
                DEST_DIR "${_macos_fw_dest}/Versions/1.0/lib/gio/modules"
                EXTENSION "dylib"
            )
        endif()
    endif()
endif()
