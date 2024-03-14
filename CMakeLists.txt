cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

include(CMakePrintHelpers)
cmake_print_variables(CMAKE_BUILD_TYPE)

#######################################################
#                   Project Info
#######################################################

include(Git)

project(QGroundControl
    VERSION 5.0
    DESCRIPTION "Ground Control App"
    HOMEPAGE_URL "http://qgroundcontrol.com/"
    LANGUAGES C CXX
)

cmake_print_variables(CMAKE_PROJECT_NAME APP_VERSION_STR)

#######################################################
#            CMake Configuration Options
#######################################################

list(APPEND CMAKE_MODULE_PATH cmake)
include(CompileOptions)
include(Qt6QGCConfiguration)
cmake_print_variables(QT_VERSION QT_MKSPEC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(MACOS TRUE)
endif()

if(LINUX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/AppDir/usr)
else()
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/staging)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR OFF)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

#######################################################
#               Qt6 Configuration
#######################################################

find_package(Qt6
    REQUIRED
    COMPONENTS
        Bluetooth
        Charts
        Concurrent
        Core
        Location
        Multimedia
        Network
        Positioning
        Quick
        QuickControls2
        QuickWidgets
        OpenGL
        Sql
        Svg
        Test
        TextToSpeech
        Widgets
        Xml
    OPTIONAL_COMPONENTS
        LinguistTools
        SerialPort
    HINTS
        ${QT_LIBRARY_HINTS}
)

qt_standard_project_setup(REQUIRES 6.6.0)

qt_policy(
    SET QTP0001 NEW
    SET QTP0002 NEW
)

qt_add_executable(${PROJECT_NAME} src/main.cc)

#######################################################
#                QML Configuration
#######################################################

set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml" CACHE PATH "Install path for QML" FORCE)
set(QML_IMPORT_PATH qml ${CMAKE_BINARY_DIR}/imports CACHE STRING "Extra QML Import Paths" FORCE)

include(FeatureSummary)

option(QGC_COMPILE_QML "Pre-compile QML files using the Qt Quick compiler." FALSE)
add_feature_info(COMPILE_QML COMPILE_QML "Pre-compile QML files using the Qt Quick compiler.")
if(QGC_COMPILE_QML)
    find_package(Qt6QuickCompiler)
    set_package_properties(Qt6QuickCompiler
        PROPERTIES
            DESCRIPTION "Pre-compile QML files using the Qt Quick compiler."
            TYPE OPTIONAL
    )
endif()

option(QGC_DEBUG_QML "Build QGroundControl with QML debugging/profiling support." FALSE)
add_feature_info(DEBUG_QML DEBUG_QML "Build QGroundControl with QML debugging/profiling support.")
if(QGC_DEBUG_QML)
    message(STATUS "To enable the QML debugger/profiler, run with: '-qmljsdebugger=port:1234'")
    add_compile_definitions(
        QMLJSDEBUGGER
        QT_QML_DEBUG
    )
endif()

#######################################################
#                QGroundControl Resources
#######################################################

set(QGC_RESOURCES
    qgcimages.qrc
    qgcresources.qrc
    qgroundcontrol.qrc
    resources/InstrumentValueIcons/InstrumentValueIcons.qrc
    src/FirmwarePlugin/APM/APMResources.qrc
    src/FirmwarePlugin/PX4/PX4Resources.qrc
)

if(WIN32)
    list(APPEND QGC_RESOURCES deploy/windows/QGroundControl.rc)
elseif(MACOS)
    set(MACOSX_BUNDLE_ICON_FILE macx.icns)
    set(app_icon_macos "resources/icons/macx.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    list(APPEND QGC_RESOURCES ${app_icon_macos})
endif()

#######################################################
#               QGroundControl Target
#######################################################

target_compile_definitions(${PROJECT_NAME}
    QT_DISABLE_DEPRECATED_BEFORE=0x060600
    QT_DEBUG_FIND_PACKAGE=ON
    QGC_APPLICATION_NAME="QGroundControl"
    QGC_ORG_NAME="QGroundControl.org"
    QGC_ORG_DOMAIN="org.qgroundcontrol"
)

option(QGC_STABLE_BUILD "Stable build option" OFF)
if(NOT QGC_STABLE_BUILD)
    target_compile_definitions(${PROJECT_NAME} QGC_DAILY_BUILD)
endif()
cmake_print_variables(QGC_STABLE_BUILD)

include(FetchContent)
include(ExternalProject)
add_subdirectory(libs)
add_subdirectory(src)
add_subdirectory(test)

set_target_properties(${PROJECT_NAME} PROPERTIES QT_RESOURCE_PREFIX "/qgc")

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.h)

if(Qt6LinguistTools_FOUND)
    file(GLOB TS_SOURCES RELATIVE ${CMAKE_SOURCE_DIR} translations/qgc_*.ts)
    # TODO: Update to new qt_add_translations form in Qt6.7
    qt_add_translations(${PROJECT_NAME} TS_FILES ${TS_SOURCES})
endif()

if(MACOS)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST deploy/mac/MacOSXBundleInfo.plist.in
    )
elseif(IOS)
    enable_language(C CXX OBJC OBJCXX)

    set_target_properties(${PROJECT_NAME}
       PROPERTIES
           QT_IOS_LAUNCH_SCREEN deploy/ios/QGCLaunchScreen.xib
    )
elseif(WIN32)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            WIN32_EXECUTABLE TRUE
    )
elseif(ANDROID)
    FetchContent_Declare(android_openssl
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
    )
    FetchContent_MakeAvailable(android_openssl)
    include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
    add_android_openssl_libraries(${PROJECT_NAME})

    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            QT_ANDROID_ABIS "${QT_ANDROID_ABIS}"
            QT_ANDROID_BUILD_ALL_ABIS OFF
            QT_ANDROID_MIN_SDK_VERSION 26
            QT_ANDROID_TARGET_SDK_VERSION 33
            QT_ANDROID_PACKAGE_SOURCE_DIR android
            QT_ANDROID_VERSION_NAME ${APP_VERSION_STR}
            # QT_ANDROID_VERSION_CODE ${CMAKE_PROJECT_VERSION}
            # QT_ANDROID_APPLICATION_ARGUMENTS
            QT_QML_ROOT_PATH ${CMAKE_SOURCE_DIR}
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release" AND ANDROID_KEYSTORE_PASSWORD)
        set(QT_ANDROID_DEPLOY_RELEASE TRUE)
        set(QT_ANDROID_SIGN_APK TRUE)
    else()
        set(QT_ENABLE_VERBOSE_DEPLOYMENT TRUE)
    endif()
endif()

#######################################################
#       Install Configuration
#######################################################

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PUBLIC QGC_INSTALL_RELEASE)
endif()

include(GNUInstallDirs)
install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    BUNDLE  DESTINATION .
)
if(NOT ANDROID)
    qt_generate_deploy_qml_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
        MACOS_BUNDLE_POST_BUILD
    )
    install(SCRIPT ${deploy_script})
endif()
install(
    DIRECTORY resources/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/qgroundcontrol
)
if(LINUX)
    install(
        FILES deploy/linux/org.mavlink.qgroundcontrol.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
    install(
        FILES resources/icons/qgroundcontrol.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps/
        RENAME org.mavlink.qgroundcontrol.png
    )
    configure_file(
        deploy/linux/org.mavlink.qgroundcontrol.metainfo.xml.in
        ${CMAKE_BINARY_DIR}/metainfo/org.mavlink.qgroundcontrol.metainfo.xml
        @ONLY
    )
    install(
        FILES ${CMAKE_BINARY_DIR}/metainfo/org.mavlink.qgroundcontrol.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo/
    )
    install(SCRIPT "cmake/CreateAppImage.cmake")
endif()
