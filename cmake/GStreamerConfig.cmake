# ============================================================================
# GStreamer Configuration
# URL configurations, exclusion lists, and install functions
# ============================================================================

# ============================================================================
# GStreamer Package URLs
# Centralized URL configuration for downloading GStreamer packages
# ============================================================================

# Helper function to get GStreamer package URLs
# Usage: gstreamer_get_package_url(<PLATFORM> <VERSION> <OUTPUT_VAR>)
#   PLATFORM: android, ios, good_plugins, monorepo
#   VERSION: GStreamer version (e.g., 1.22.12)
#   OUTPUT_VAR: Variable to store the URL
function(gstreamer_get_package_url PLATFORM VERSION OUTPUT_VAR)
    if(PLATFORM STREQUAL "android")
        set(_url "https://gstreamer.freedesktop.org/data/pkg/android/${VERSION}/gstreamer-1.0-android-universal-${VERSION}.tar.xz")
    elseif(PLATFORM STREQUAL "ios")
        set(_url "https://gstreamer.freedesktop.org/data/pkg/ios/${VERSION}/gstreamer-1.0-devel-${VERSION}-ios-universal.pkg")
    elseif(PLATFORM STREQUAL "good_plugins")
        set(_url "https://gstreamer.freedesktop.org/src/gst-plugins-good/gst-plugins-good-${VERSION}.tar.xz")
    elseif(PLATFORM STREQUAL "monorepo")
        # GitLab monorepo archive - use specific version tag for stability
        set(_url "https://gitlab.freedesktop.org/gstreamer/gstreamer/-/archive/${VERSION}/gstreamer-${VERSION}.tar.gz")
    else()
        message(FATAL_ERROR "gstreamer_get_package_url: Unknown platform '${PLATFORM}'")
    endif()

    set(${OUTPUT_VAR} "${_url}" PARENT_SCOPE)
endfunction()

# Helper function to get SHA256 checksum URL for a GStreamer package
# Usage: gstreamer_get_checksum_url(<PLATFORM> <VERSION> <OUTPUT_VAR>)
function(gstreamer_get_checksum_url PLATFORM VERSION OUTPUT_VAR)
    gstreamer_get_package_url(${PLATFORM} ${VERSION} _pkg_url)
    set(${OUTPUT_VAR} "${_pkg_url}.sha256sum" PARENT_SCOPE)
endfunction()

# Helper function to determine the recommended patch version for a GStreamer minor version
# Usage: gstreamer_get_recommended_version(<MAJOR> <MINOR> <OUTPUT_VAR>)
#   Returns the latest stable patch version for the given major.minor
function(gstreamer_get_recommended_version MAJOR MINOR OUTPUT_VAR)
    # Latest stable patch versions for each minor release
    if(MINOR EQUAL 20)
        set(_patch 7)
    elseif(MINOR EQUAL 22)
        set(_patch 12)
    elseif(MINOR EQUAL 24)
        set(_patch 13)
    elseif(MINOR EQUAL 26)
        set(_patch 6)
    else()
        set(_patch 0)
    endif()

    set(${OUTPUT_VAR} "${MAJOR}.${MINOR}.${_patch}" PARENT_SCOPE)
endfunction()

# ============================================================================
# GStreamer Plugin Exclusion Lists
# Plugins excluded from installation (QGC only needs video streaming)
# These are plugin names without prefix (libgst) or extension (.so/.dll)
#
# Users can override by setting QGC_GSTREAMER_EXCLUDED_PLUGINS before including
# this file. Set to empty string to disable all exclusions.
# ============================================================================
if(DEFINED QGC_GSTREAMER_EXCLUDED_PLUGINS)
    set(GSTREAMER_EXCLUDED_PLUGINS ${QGC_GSTREAMER_EXCLUDED_PLUGINS})
else()

set(GSTREAMER_EXCLUDED_PLUGINS
    # Audio plugins (QGC is video-only)
    a52dec
    adpcmdec
    adpcmenc
    alaw
    alsa
    amrnb
    amrwbdec
    audiobuffersplit
    audioconvert
    audiofx
    audiofxbad
    audiolatency
    audiomixer
    audiomixmatrix
    audioparsers
    audiorate
    audioresample
    audiotestsrc
    audiovisualizers
    bs2b
    dsd
    dtmf
    dtsdec
    equalizer
    espeak
    faad
    fdkaac
    flac
    fluidsynthmidi
    gme
    gsm
    jack
    ladspa
    lame
    lc3
    ldac
    level
    lv2
    midi
    modplug
    mpg123
    mulaw
    openaptx
    openmpt
    opus
    opusparse
    pipewire
    pocketsphinx
    pulseaudio
    removesilence
    replaygain
    sbc
    sid
    siren
    soundtouch
    speex
    spandsp
    spectrum
    twolame
    voaacenc
    voamrwbenc
    volume
    vorbis
    wavenc
    wavpack
    wavparse
    wildmidi

    # DVD/Bluray/CD (not needed for drone video)
    cdio
    cdparanoia
    dvdlpcmdec
    dvdread
    dvdspu
    dvdsub
    resindvd

    # Image formats (QGC doesn't process images via GStreamer)
    gdkpixbuf
    jpeg
    jpegformat
    openexr
    openjpeg
    png
    pnm
    webp

    # Subtitle/text rendering
    assrender
    closedcaption
    dvbsubenc
    dvbsuboverlay
    kate
    pango
    subenc
    subparse
    teletext
    ttmlsubs

    # Legacy/specialized hardware (non-video)
    1394
    dc1394
    decklink
    dvb
    openal
    openni2
    oss4
    ossaudio
    shm
    winks

    # Effects/visualization (non-video)
    aasink
    accurip
    adder
    cacasink
    cutter
    frei0r
    freeverb
    goom
    goom2k1
    inter
    interleave
    legacyrawparse
    libvisual
    monoscope
    qroverlay
    rfbsrc
    rsvg
    segmentclip
    shapewipe
    smpte
    speed
    zbar
    zxing

    # Debug/development tools
    analyticsoverlay
    basedebug
    codec2json
    codecalpha
    codectimestamper
    coretracers
    debug
    debugutilsbad
    festival
    flite
    netsim

    # Unused container formats
    aiff
    asf
    asfmux
    auparse
    avi
    flv
    flxdec
    icydemux
    mxf
    ogg
    realmedia

    # Streaming protocols not typically used for drone video
    # Note: HLS/DASH kept for IP camera compatibility (adaptivedemux2, hls, dash, soup)
    mpegtsmux
    mse
    neonhttpsrc
    rtmp
    rtmp2
    rtpmanagerbad
    rtponvif
    smooth
    smoothstreaming
    srt
    srtp

    # Bluetooth/fingerprinting
    bluez
    chromaprint

    # Encryption/DRM
    aes
    dtls

    # Misc unused
    apetag
    avtp
    bz2
    gdp
    gtk
    gtkwayland
    id3demux
    id3tag
    insertbin
    ipcpipeline
    libcluttergst3
    multipart
    musepack
    nice
    pbtypes
    pcapparse
    peadapter
    peautogain
    peconvolver
    pecrystalizer
    proxy
    python
    rist
    rtspclientsink
    sctp
    shout2
    sndfile
    switchbin
    taglib
    unixfd
    webrtc
    webrtcdsp
    xingmux
)

endif() # QGC_GSTREAMER_EXCLUDED_PLUGINS

# ============================================================================
# GStreamer Base Libraries Exclusion List
# Libraries not needed for video streaming (primarily affects Windows)
# Users can override by setting QGC_GSTREAMER_EXCLUDED_LIBS
# ============================================================================
if(DEFINED QGC_GSTREAMER_EXCLUDED_LIBS)
    set(GSTREAMER_EXCLUDED_LIBS ${QGC_GSTREAMER_EXCLUDED_LIBS})
else()

set(GSTREAMER_EXCLUDED_LIBS
    # Audio libraries (QGC is video-only)
    gstaudio-1.0
    gstbadaudio-1.0
    gstfft-1.0

    # Unused features
    gstadaptivedemux-1.0
    gstanalytics-1.0
    gstcheck-1.0
    gstinsertbin-1.0
    gstisoff-1.0
    gstmse-1.0
    gstopencv-1.0
    gstplay-1.0
    gstplayer-1.0
    gstrtspserver-1.0
    gstsctp-1.0
    gsturidownloader-1.0
    gstwebrtc-1.0
    gstwebrtcnice-1.0
)

endif() # QGC_GSTREAMER_EXCLUDED_LIBS

# ============================================================================
# GIO Modules Allowlist
# Only these GIO modules are needed (TLS/SSL support for network connections)
# ============================================================================
set(GSTREAMER_GIO_MODULES_ALLOWED
    gioopenssl
    giognutls
)

# ============================================================================
# Function: gstreamer_install_gio_modules
# Install only required GIO modules
#
# Arguments:
#   SOURCE_DIR  - Directory containing GIO modules (e.g., lib/gio/modules)
#   DEST_DIR    - Installation destination
#   EXTENSION   - File extension (so or dll)
# ============================================================================
function(gstreamer_install_gio_modules)
    cmake_parse_arguments(ARG "" "SOURCE_DIR;DEST_DIR;EXTENSION" "" ${ARGN})

    # Validate source directory exists
    if(NOT EXISTS "${ARG_SOURCE_DIR}")
        message(WARNING "gstreamer_install_gio_modules: SOURCE_DIR does not exist: ${ARG_SOURCE_DIR}")
        return()
    endif()

    # Glob all modules
    file(GLOB all_modules "${ARG_SOURCE_DIR}/*.${ARG_EXTENSION}*")

    # Filter to only allowed modules
    set(modules_to_install "")
    foreach(module_path IN LISTS all_modules)
        get_filename_component(module_name "${module_path}" NAME)

        # Check if this module is in the allowlist
        # Match libgioopenssl.so (Linux) or gioopenssl.dll (Windows)
        foreach(allowed IN LISTS GSTREAMER_GIO_MODULES_ALLOWED)
            if(module_name MATCHES "^(lib)?${allowed}([^a-zA-Z]|$)")
                list(APPEND modules_to_install "${module_path}")
                break()
            endif()
        endforeach()
    endforeach()

    # Install filtered modules
    if(modules_to_install)
        install(FILES ${modules_to_install} DESTINATION "${ARG_DEST_DIR}")
    endif()
endfunction()

# ============================================================================
# Function: gstreamer_install_plugins
# Install GStreamer plugins with exclusions applied
#
# Arguments:
#   SOURCE_DIR  - Directory containing plugins
#   DEST_DIR    - Installation destination
#   EXTENSION   - File extension (so or dll)
#   PREFIX      - File prefix (libgst for Linux, gst for Windows)
# ============================================================================
function(gstreamer_install_plugins)
    cmake_parse_arguments(ARG "" "SOURCE_DIR;DEST_DIR;EXTENSION;PREFIX" "" ${ARGN})

    # Validate source directory exists
    if(NOT EXISTS "${ARG_SOURCE_DIR}")
        message(WARNING "gstreamer_install_plugins: SOURCE_DIR does not exist: ${ARG_SOURCE_DIR}")
        return()
    endif()

    # Glob all plugins
    file(GLOB all_plugins "${ARG_SOURCE_DIR}/${ARG_PREFIX}*.${ARG_EXTENSION}")

    # Filter out excluded plugins
    set(plugins_to_install "")
    foreach(plugin_path IN LISTS all_plugins)
        get_filename_component(plugin_name "${plugin_path}" NAME)

        # Check if this plugin should be excluded
        # Regex matches prefix + plugin name + non-alpha char or end of name (before extension)
        set(exclude FALSE)
        foreach(excluded IN LISTS GSTREAMER_EXCLUDED_PLUGINS)
            if(plugin_name MATCHES "^${ARG_PREFIX}${excluded}([^a-zA-Z]|$)")
                set(exclude TRUE)
                break()
            endif()
        endforeach()

        if(NOT exclude)
            list(APPEND plugins_to_install "${plugin_path}")
        endif()
    endforeach()

    # Install filtered plugins
    if(plugins_to_install)
        install(FILES ${plugins_to_install} DESTINATION "${ARG_DEST_DIR}")
    endif()
endfunction()

# ============================================================================
# Function: gstreamer_install_libs
# Install GStreamer libraries with exclusions applied (for Windows)
#
# Arguments:
#   SOURCE_DIR  - Directory containing libraries (e.g., bin/ on Windows)
#   DEST_DIR    - Installation destination
#   EXTENSION   - File extension (dll)
# ============================================================================
function(gstreamer_install_libs)
    cmake_parse_arguments(ARG "" "SOURCE_DIR;DEST_DIR;EXTENSION" "" ${ARGN})

    # Validate source directory exists
    if(NOT EXISTS "${ARG_SOURCE_DIR}")
        message(WARNING "gstreamer_install_libs: SOURCE_DIR does not exist: ${ARG_SOURCE_DIR}")
        return()
    endif()

    # Glob all DLLs
    file(GLOB all_libs "${ARG_SOURCE_DIR}/*.${ARG_EXTENSION}")

    # Filter out excluded libraries
    set(libs_to_install "")
    foreach(lib_path IN LISTS all_libs)
        get_filename_component(lib_name "${lib_path}" NAME)

        # Check if this is a GStreamer library that should be excluded
        # Match libgstfoo-1.0.dll or gstfoo-1.0.dll patterns
        set(exclude FALSE)
        foreach(excluded IN LISTS GSTREAMER_EXCLUDED_LIBS)
            if(lib_name MATCHES "(lib)?${excluded}([^a-zA-Z]|$)")
                set(exclude TRUE)
                break()
            endif()
        endforeach()

        if(NOT exclude)
            list(APPEND libs_to_install "${lib_path}")
        endif()
    endforeach()

    # Install filtered libraries
    if(libs_to_install)
        install(FILES ${libs_to_install} DESTINATION "${ARG_DEST_DIR}")
    endif()
endfunction()
