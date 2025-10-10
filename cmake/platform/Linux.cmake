# ==============================================================================
# QGroundControl Linux Platform Configuration
# ==============================================================================
# Platform-specific settings for Linux builds
# ==============================================================================

if(NOT LINUX)
    message(FATAL_ERROR "QGC: Invalid Platform - This file is for Linux only")
    return()
endif()

message(STATUS "QGC: Configuring for Linux platform")

# ==============================================================================
# Linux-specific Compiler and Linker Flags
# ==============================================================================

# Enable security hardening features
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    # Stack protection
    $<$<CONFIG:Release>:-fstack-protector-strong>
    $<$<CONFIG:RelWithDebInfo>:-fstack-protector-strong>

    # Additional warnings for Linux
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wpedantic>
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    # Security: Full RELRO (RELocation Read-Only)
    $<$<CONFIG:Release>:LINKER:-z,relro,-z,now>
    $<$<CONFIG:RelWithDebInfo>:LINKER:-z,relro,-z,now>

    # Strip symbols in Release builds for smaller binaries
    $<$<CONFIG:Release>:LINKER:--strip-all>
)

# ==============================================================================
# Linux Desktop Integration
# ==============================================================================

# Set application metadata for Linux desktop environments
set_target_properties(${CMAKE_PROJECT_NAME}
    PROPERTIES
        OUTPUT_NAME "${CMAKE_PROJECT_NAME}"
        RUNTIME_OUTPUT_NAME "${CMAKE_PROJECT_NAME}"
)

# ==============================================================================
# Linux-specific Libraries and Dependencies
# ==============================================================================

# Check for required Linux system libraries
include(CheckLibraryExists)

# Check for pthread (usually available but good to verify)
find_package(Threads REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Threads::Threads)

# Check for libdl (dynamic linking library)
check_library_exists(dl dlopen "" HAVE_LIBDL)
if(HAVE_LIBDL)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})
endif()

# ==============================================================================
# Linux Distribution Detection
# ==============================================================================

if(EXISTS "/etc/os-release")
    file(STRINGS "/etc/os-release" _os_release_content)
    foreach(_line ${_os_release_content})
        if(_line MATCHES "^ID=(.+)$")
            set(LINUX_DISTRO_ID ${CMAKE_MATCH_1})
            string(REPLACE "\"" "" LINUX_DISTRO_ID "${LINUX_DISTRO_ID}")
        endif()
        if(_line MATCHES "^VERSION_ID=(.+)$")
            set(LINUX_DISTRO_VERSION ${CMAKE_MATCH_1})
            string(REPLACE "\"" "" LINUX_DISTRO_VERSION "${LINUX_DISTRO_VERSION}")
        endif()
    endforeach()
    message(STATUS "QGC: Detected Linux distribution: ${LINUX_DISTRO_ID} ${LINUX_DISTRO_VERSION}")
endif()

# ==============================================================================
# Linux-specific Feature Detection
# ==============================================================================

# Detect Wayland support
if(TARGET Qt6::WaylandClient)
    message(STATUS "QGC: Wayland support available")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE QGC_WAYLAND_ENABLED)
endif()

# Set RPATH for installed binaries
set_target_properties(${CMAKE_PROJECT_NAME}
    PROPERTIES
        INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH_USE_LINK_PATH ON
)

message(STATUS "QGC: Linux platform configuration complete")
