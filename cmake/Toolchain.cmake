# ==============================================================================
# C++ Standard Configuration
# ==============================================================================
# C++20 is required for modern language features and better Qt6 integration

set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard version")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure C++20 is actually enforced
if(NOT CMAKE_CXX_STANDARD EQUAL 20)
    message(WARNING "QGC: C++20 is required, but CMAKE_CXX_STANDARD is set to ${CMAKE_CXX_STANDARD}")
endif()

# ==============================================================================
# Qt Auto-Generation Tools
# ==============================================================================

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ==============================================================================
# Build System Configuration
# ==============================================================================

# Enable colored diagnostics for better readability
set(CMAKE_COLOR_DIAGNOSTICS ON)

# Export compile commands for IDE integration and tooling (clangd, cppcheck, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include current directory for generated files
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Unity builds can speed up compilation significantly but may hide some issues
# Uncomment to enable:
# set(CMAKE_UNITY_BUILD ON)
# set(CMAKE_UNITY_BUILD_BATCH_SIZE 16)

# ==============================================================================
# Optimization and Security Features
# ==============================================================================

# Enable Position Independent Executable (security feature)
qgc_enable_pie()

# Enable Link-Time Optimization for Release builds
qgc_enable_ipo()

# ==============================================================================
# Compiler-Specific Configuration
# ==============================================================================

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    # Use modern fast linker on Linux
    if(NOT APPLE)
        qgc_set_linker()
    endif()

    # Enable thin LTO for faster linking
    add_link_options("$<$<CONFIG:Release>:-flto=thin>")

    # Additional compiler warnings for better code quality
    add_compile_options(
        $<$<CXX_COMPILER_ID:Clang>:-Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-parameter>
    )

elseif(MSVC)
    # MSVC-specific optimization settings
    add_link_options("$<$<CONFIG:Release>:/LTCG:INCREMENTAL>")

    # Use embedded debug information for better debugging experience
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:Embedded>")

    # Use DLL runtime library for consistent runtime linking
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    # Additional MSVC optimizations
    add_compile_options(
        $<$<CONFIG:Release>:/Ot>  # Favor speed
        $<$<CONFIG:Release>:/Oi>  # Enable intrinsics
        $<$<CONFIG:Release>:/GL>  # Whole program optimization
    )
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(LINUX)
        # set(ENV{DESTDIR} "${CMAKE_BINARY_DIR}/staging")
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/AppDir/usr" CACHE PATH "Install path prefix for AppImage" FORCE)
    else()
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/staging" CACHE PATH "Install path prefix" FORCE)
    endif()
endif()

if(CMAKE_CROSSCOMPILING)
    if(NOT IS_DIRECTORY ${QT_HOST_PATH})
        message(FATAL_ERROR "You need to set QT_HOST_PATH to cross compile Qt.")
    endif()

    if(ANDROID)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    endif()
endif()

if(APPLE AND NOT IOS)
    set(MACOS TRUE)
endif()
