#[[============================================================================
Function: qgc_set_qt_resource_alias

Description:
    Sets Qt resource aliases for files, using their base filename as the alias.
    This simplifies resource access in QML/C++ code.

Parameters:
    ARGN - List of resource files to set aliases for

Example:
    qgc_set_qt_resource_alias(
        ${CMAKE_SOURCE_DIR}/resources/icon.png
        ${CMAKE_SOURCE_DIR}/resources/logo.svg
    )
============================================================================]]
function(qgc_set_qt_resource_alias)
    foreach(resource_file IN LISTS ARGN)
        if(NOT EXISTS "${resource_file}")
            message(WARNING "QGC: Resource file does not exist: ${resource_file}")
            continue()
        endif()

        get_filename_component(alias "${resource_file}" NAME)
        set_source_files_properties("${resource_file}"
            PROPERTIES
                QT_RESOURCE_ALIAS "${alias}"
        )
    endforeach()
endfunction()

#[[============================================================================
Function: qgc_config_caching

Description:
    Configures compiler caching using ccache or sccache to speed up rebuilds.
    Automatically detects available cache tools and configures them optimally.

Parameters:
    None

Notes:
    - Prefers ccache if available, falls back to sccache
    - Sets appropriate environment variables for optimal performance
    - Configures compiler launchers for C and C++

Example:
    if(QGC_USE_CACHE)
        qgc_config_caching()
    endif()
============================================================================]]
function(qgc_config_caching)
    # Internal validator function for cache tools
    function(_qgc_verify_cache_tool _ok _path)
        execute_process(
            COMMAND "${_path}" --version
            RESULT_VARIABLE _res
            OUTPUT_QUIET ERROR_QUIET
        )
        if(NOT _res EQUAL 0)
            set(${_ok} FALSE PARENT_SCOPE)
        else()
            set(${_ok} TRUE PARENT_SCOPE)
        endif()
    endfunction()

    find_program(QGC_CACHE_PROGRAM
                 NAMES ccache sccache
                 VALIDATOR _qgc_verify_cache_tool
                 DOC "Compiler cache program")

    if(QGC_CACHE_PROGRAM)
        get_filename_component(_cache_tool "${QGC_CACHE_PROGRAM}" NAME_WE)
        message(STATUS "QGC: using ${_cache_tool} (${QGC_CACHE_PROGRAM})")
        string(TOLOWER "${_cache_tool}" _cache_tool)

        if(_cache_tool STREQUAL "ccache")
            # set(ENV{CCACHE_CONFIGPATH} "${CMAKE_SOURCE_DIR}/tools/ccache.conf")
            # set(ENV{CCACHE_DIR} "${CMAKE_SOURCE_DIR}/.ccache")
            set(ENV{CCACHE_BASEDIR} "${CMAKE_SOURCE_DIR}")
            set(ENV{CCACHE_COMPRESSLEVEL "5")
            set(ENV{CCACHE_SLOPPINESS} "pch_defines,time_macros,include_file_mtime,include_file_ctime")
            # set(ENV{CCACHE_NOHASHDIR} "true")
            if(APPLE)
                set(ENV{CCACHE_COMPILERCHECK} "content")
            endif()
            # set(ENV{CCACHE_MAXSIZE} "1G")
        elseif(_cache_tool STREQUAL "sccache")
            # set(ENV{SCCACHE_PATH} "")
            # set(ENV{SCCACHE_DIR} "")
            # set(ENV{SCCACHE_SERVER_PORT} "")
            # set(ENV{SCCACHE_SERVER_UDS} "")
            # set(ENV{SCCACHE_IGNORE_SERVER_IO_ERROR} "")
            # set(ENV{SCCACHE_C_CUSTOM_CACHE_BUSTER} "")
            # set(ENV{SCCACHE_RECACHE} "")
            # set(ENV{SCCACHE_ERROR_LOG} "")
            # set(ENV{SCCACHE_LOG} "")
            # set(ENV{SCCACHE_CACHE_SIZE} "")
            # set(ENV{SCCACHE_IDLE_TIMEOUT} "")
        else()
            return()
        endif()

        set(CMAKE_C_COMPILER_LAUNCHER "${QGC_CACHE_PROGRAM}" CACHE STRING "C compiler launcher")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${QGC_CACHE_PROGRAM}" CACHE STRING "CXX compiler launcher")
        # set(CMAKE_C_LINKER_LAUNCHER "${QGC_CACHE_PROGRAM}" CACHE STRING "C linker cache used")
        # set(CMAKE_CXX_LINKER_LAUNCHER "${QGC_CACHE_PROGRAM}" CACHE STRING "CXX linker cache used")

        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            add_compile_options(-Xclang -fno-pch-timestamp)
        endif()
    else()
        message(WARNING "QGC: no ccache/sccache found – building without a compiler cache")
    endif()
endfunction()

#[[============================================================================
Function: qgc_set_linker

Description:
    Automatically selects the fastest available linker for the build.
    Tries modern linkers in order of preference: mold > lld > gold
    Falls back to the system default linker if none are available.

Parameters:
    None

Notes:
    - mold: Fastest modern linker for Linux
    - lld: LLVM linker, fast and cross-platform
    - gold: GNU gold linker, faster than traditional ld
    - Only applies to non-Apple platforms (macOS uses ld64)

Example:
    if(NOT APPLE)
        qgc_set_linker()
    endif()
============================================================================]]
function(qgc_set_linker)
    include(CheckLinkerFlag)

    # Try linkers in order of preference (fastest first)
    foreach(_ld mold lld gold)
        set(_flag "LINKER:-fuse-ld=${_ld}")
        check_linker_flag(CXX "${_flag}" HAVE_LD_${_ld})

        if(HAVE_LD_${_ld})
            add_link_options("${_flag}")
            set(QGC_LINKER "${_ld}" PARENT_SCOPE)
            message(STATUS "QGC: using ${_ld} linker (flag ${_flag})")
            return()
        endif()
    endforeach()

    message(STATUS "QGC: no mold / lld / gold found – using default linker")
endfunction()

#[[============================================================================
Function: qgc_enable_pie

Description:
    Enables Position Independent Executable (PIE) if supported by the toolchain.
    PIE is a security feature that enables ASLR (Address Space Layout Randomization).

Parameters:
    None

Notes:
    - PIE is recommended for security hardening
    - Required on some modern Linux distributions
    - May have small performance overhead on 32-bit systems

Example:
    qgc_enable_pie()
============================================================================]]
function(qgc_enable_pie)
    include(CheckPIESupported)
    check_pie_supported(OUTPUT_VARIABLE _output)

    if(CMAKE_C_LINK_PIE_SUPPORTED)
        set(CMAKE_POSITION_INDEPENDENT_CODE ON PARENT_SCOPE)
        message(STATUS "QGC: PIE (Position Independent Executable) enabled")
    else()
        message(WARNING "QGC: PIE not supported by toolchain: ${_output}")
    endif()
endfunction()

#[[============================================================================
Function: qgc_enable_ipo

Description:
    Enables Inter-Procedural Optimization (IPO/LTO) for Release builds.
    IPO can significantly improve runtime performance at the cost of longer
    link times.

Parameters:
    None

Notes:
    - Only enabled for Release builds to avoid slow Debug builds
    - Also known as Link-Time Optimization (LTO)
    - Can reduce binary size and improve performance by 5-15%
    - Requires significant memory during linking

Example:
    qgc_enable_ipo()
============================================================================]]
function(qgc_enable_ipo)
    if(CMAKE_BUILD_TYPE MATCHES "^(Release|RelWithDebInfo|MinSizeRel)$")
        include(CheckIPOSupported)
        check_ipo_supported(RESULT _result OUTPUT _output)

        if(_result)
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE PARENT_SCOPE)
            message(STATUS "QGC: IPO/LTO (Link-Time Optimization) enabled for ${CMAKE_BUILD_TYPE}")
        else()
            message(STATUS "QGC: IPO/LTO not supported by toolchain: ${_output}")
        endif()
    else()
        message(STATUS "QGC: IPO/LTO disabled for ${CMAKE_BUILD_TYPE} build")
    endif()
endfunction()
