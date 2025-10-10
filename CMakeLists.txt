# ==============================================================================
# QGroundControl CMake Configuration
# ==============================================================================
# Minimum CMake 3.25 required for:
# - CMake 3.25: SYSTEM keyword in find_package, block() scoping improvements
# - CMake 3.24: CMAKE_COMPILE_WARNING_AS_ERROR, workflow presets
# - CMake 3.21: Different link library groups
# ==============================================================================

cmake_minimum_required(VERSION 3.25...3.32 FATAL_ERROR)

# ==============================================================================
# CMake Module Path Configuration
# ==============================================================================

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_SOURCE_DIR}/cmake/CPack
    ${CMAKE_SOURCE_DIR}/cmake/find-modules
    ${CMAKE_SOURCE_DIR}/cmake/install
    ${CMAKE_SOURCE_DIR}/cmake/install/CPack
    ${CMAKE_SOURCE_DIR}/cmake/modules
    ${CMAKE_SOURCE_DIR}/cmake/platform
)

include(Helpers)

# ==============================================================================
# Build Type Configuration
# ==============================================================================

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel" CACHE STRING "Configuration types" FORCE)
endif()

set(CMAKE_REQUIRED_QUIET ON)
set(CMAKE_POLICY_VERSION_MINIMUM ${CMAKE_MINIMUM_REQUIRED_VERSION})

#######################################################
#                Custom Build Configuration
#######################################################

include(CustomOptions)

if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/custom")
    message(STATUS "QGC: Enabling custom build")
    set(QGC_CUSTOM_BUILD ON)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/custom/cmake")
    include(CustomOverrides)
endif()

if(NOT QGC_BUILD_TESTING)
    set(BUILD_TESTING OFF CACHE INTERNAL "" FORCE)
endif()

#######################################################
#                   Project Info
#######################################################

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")
    # set(CMAKE_OSX_SYSROOT "iphoneos")
    if(QGC_MACOS_UNIVERSAL_BUILD)
        set(CMAKE_OSX_ARCHITECTURES "x86_64h;arm64") # CMAKE_APPLE_SILICON_PROCESSOR
    endif()
endif()

include(Git)

if(QGC_USE_CACHE)
    qgc_config_caching()
endif()

project(${QGC_APP_NAME}
    VERSION ${QGC_APP_VERSION}
    DESCRIPTION ${QGC_APP_DESCRIPTION}
    HOMEPAGE_URL "https://${QGC_ORG_DOMAIN}"
    LANGUAGES C CXX
)

#######################################################
#            CMake Configuration Options
#######################################################

include(GNUInstallDirs)
include(FetchContent)
include(CMakePrintHelpers)

include(CPM)
if(NOT CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_BINARY_DIR}/cpm_modules")
endif()

include(Toolchain)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

# ==============================================================================
# CMake Policy Configuration
# ==============================================================================
# Set modern policies for better behavior and compatibility

# CMP0168: Allow FetchContent to use DOWNLOAD_EXTRACT_TIMESTAMP
if(POLICY CMP0168)
    cmake_policy(SET CMP0168 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0168 NEW)
endif()

# CMP0141: MSVC debug information format flags are selected by an abstraction
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0141 NEW)
endif()

# CMP0075: Include file check macros honor CMAKE_REQUIRED_LIBRARIES
if(POLICY CMP0075)
    cmake_policy(SET CMP0075 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)
endif()

# CMP0091: MSVC runtime library flags are selected by an abstraction
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
endif()

# CMP0092: Don't add -W3 warning flag by default with MSVC
if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0092 NEW)
endif()

# CMP0117: AUTOMOC, AUTOUIC, AUTORCC generate the .moc files in the build directory
if(POLICY CMP0117)
    cmake_policy(SET CMP0117 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0117 NEW)
endif()

# ==============================================================================
# Qt6 Configuration
# ==============================================================================
# Qt6 6.8.3+ is required for QGroundControl
# Using SYSTEM to reduce warnings from Qt headers

find_package(Qt6
    ${QGC_QT_MINIMUM_VERSION}...${QGC_QT_MAXIMUM_VERSION}
    REQUIRED
    COMPONENTS
        Charts
        Concurrent
        Core
        Core5Compat
        Gui
        LinguistTools
        Location
        Multimedia
        Network
        Positioning
        Qml
        QmlIntegration
        Quick
        QuickControls2
        QuickWidgets
        Sensors
        Sql
        Svg
        TextToSpeech
        Widgets
        Xml
    OPTIONAL_COMPONENTS
        Bluetooth
        MultimediaQuickPrivate
        OpenGL
        Quick3D
        SerialPort
        Test
    GLOBAL # Make Qt targets available globally
)

# Platform-specific Qt components
if(LINUX)
    find_package(Qt6 COMPONENTS WaylandClient QUIET)
    if(Qt6WaylandClient_FOUND)
        message(STATUS "QGC: Wayland support enabled")
    endif()
endif()

# Validate Qt version
if(Qt6_VERSION VERSION_LESS ${QGC_QT_MINIMUM_VERSION})
    message(FATAL_ERROR "Qt ${Qt6_VERSION} found but ${QGC_QT_MINIMUM_VERSION} is required")
endif()

# Qt standard project setup with modern features
qt_standard_project_setup(
    REQUIRES ${QGC_QT_MINIMUM_VERSION}
    SUPPORTS_UP_TO ${QGC_QT_MAXIMUM_VERSION}
    I18N_SOURCE_LANGUAGE en
    I18N_TRANSLATED_LANGUAGES zh_CN zh_TW de es fr it ja ko pt ru tr
)

# Qt policies for modern behavior
# QTP0001: Ensure Qt resource files are processed correctly
# QTP0002: Disable deprecated APIs warnings
# QTP0003: Ensure consistent behavior with QML module imports
# QTP0004: Use modern CMake integration for Qt
# QTP0005: Enable strict mode for Qt deployments
qt_policy(
    SET QTP0001 NEW
    SET QTP0002 NEW
    SET QTP0003 NEW
    SET QTP0004 NEW
    SET QTP0005 NEW
)

# Feature detection for optional Qt components
set(QGC_QT_FEATURES "")
if(TARGET Qt6::Bluetooth)
    list(APPEND QGC_QT_FEATURES "Bluetooth")
endif()
if(TARGET Qt6::SerialPort)
    list(APPEND QGC_QT_FEATURES "SerialPort")
endif()
if(TARGET Qt6::Quick3D)
    list(APPEND QGC_QT_FEATURES "Quick3D")
endif()
if(TARGET Qt6::Test)
    list(APPEND QGC_QT_FEATURES "Test")
endif()
if(QGC_QT_FEATURES)
    message(STATUS "QGC: Optional Qt components found: ${QGC_QT_FEATURES}")
endif()

#######################################################
#                Custom Build Configuration
#######################################################

if(QGC_CUSTOM_BUILD)
    add_subdirectory(custom)
endif()

#######################################################
#                QGroundControl Resources
#######################################################

# Note: Adding Resources to Library instead requires using Q_INIT_RESOURCE(qgcresources)

list(APPEND QGC_RESOURCES
    ${CMAKE_SOURCE_DIR}/qgcimages.qrc
    ${CMAKE_SOURCE_DIR}/qgcresources.qrc
)

list(APPEND QGC_RESOURCES
    ${CMAKE_SOURCE_DIR}/resources/InstrumentValueIcons/InstrumentValueIcons.qrc
)

#######################################################
#               QGroundControl Target
#######################################################

qt_add_executable(${CMAKE_PROJECT_NAME}
    WIN32
    MACOSX_BUNDLE
    ${QGC_RESOURCES}
)

if(WIN32)
    include(Windows)
elseif(APPLE)
    include(Apple)
elseif(ANDROID)
    include(Android)
endif()

qt_add_qml_module(${CMAKE_PROJECT_NAME}
    URI QGC
    VERSION 1.0
    RESOURCE_PREFIX /qml
    IMPORTS
        QtQuick
        QtQuick.Controls
        QtQuick.Dialogs
        QtQuick.Layouts
)

add_subdirectory(src)
if(QGC_BUILD_TESTING)
    add_subdirectory(test)
    set_property(DIRECTORY test PROPERTY QT_EXCLUDE_FROM_TRANSLATION ON)
endif()

file(GLOB TS_SOURCES ${CMAKE_SOURCE_DIR}/translations/qgc_*.ts)
set_source_files_properties(${TS_SOURCES} PROPERTIES OUTPUT_LOCATION "${CMAKE_BINARY_DIR}/i18n")

qt_add_translations(${CMAKE_PROJECT_NAME}
    TS_FILES ${TS_SOURCES}
    RESOURCE_PREFIX "/"
    LUPDATE_OPTIONS -no-obsolete
    TS_FILE_DIR "${CMAKE_SOURCE_DIR}/translations"
    TS_FILE_BASE ${CMAKE_PROJECT_NAME}
)

qgc_set_qt_resource_alias(${CMAKE_SOURCE_DIR}/resources/qtquickcontrols2.conf)

qt_add_resources(${CMAKE_PROJECT_NAME} "qgcresources_cmake"
    PREFIX "/"
    FILES
        "${CMAKE_SOURCE_DIR}/resources/qtquickcontrols2.conf"
)

# cmake_print_variables(QT_ALL_PLUGIN_TYPES_FOUND_VIA_FIND_PACKAGE)
qt_import_plugins(${CMAKE_PROJECT_NAME}
    INCLUDE Qt6::QSvgPlugin
    EXCLUDE_BY_TYPE geoservices
    INCLUDE_BY_TYPE sqldrivers Qt6::QSQLiteDriverPlugin
    # INCLUDE_BY_TYPE styles Qt6::qtquickcontrols2basicstyleplugin Qt6::qtquickcontrols2basicstyleimplplugin
)

if(LINUX)
    qt_import_plugins(${CMAKE_PROJECT_NAME} 
        INCLUDE
            Qt6::QWaylandIntegrationPlugin
            Qt6::QXcbIntegrationPlugin
            Qt6::QEglFSIntegrationPlugin
            Qt6::QWaylandEglPlatformIntegrationPlugin
    )
endif()

include(Install)

include(PrintSummary)
