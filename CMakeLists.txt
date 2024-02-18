cmake_minimum_required(VERSION 3.21.1 FATAL_ERROR)

#######################################################
#                   Project Info
#######################################################

project(QGroundControl LANGUAGES C CXX)

include(CMakePrintHelpers)
include(FetchContent)
include(ExternalProject)
include(FeatureSummary)
include(GNUInstallDirs)

#######################################################
#            CMake Configuration Options
#######################################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR OFF)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;RelWithDebInfo;MinSizeRel;Coverage")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

option(STABLE_BUILD "Stable build option" OFF)
if (STABLE_BUILD)
    message(STATUS "Stable Build")
else ()
    message(STATUS "Daily Build")
    add_compile_definitions(DAILY_BUILD)
endif ()

if(ANDROID)
    set(MOBILE TRUE)
endif()

#######################################################
#               Qt6 Configuration
#######################################################

find_package(Qt6 REQUIRED COMPONENTS Core Qml)

qt_standard_project_setup(REQUIRES 6.6.0 SUPPORTS_UP_TO 6.6.2)

set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml" CACHE PATH "Install path for QML" FORCE)
#set(QML_IMPORT_PATH "${CMAKE_BINARY_DIR}/qml" CACHE STRING "Extra QML Import Paths" FORCE)

qt_policy(
    SET QTP0001 NEW
    SET QTP0002 NEW
)

option(DEBUG_QML "Build QGroundControl with QML debugging/profiling support." FALSE)
add_feature_info(DEBUG_QML DEBUG_QML "Build QGroundControl with QML debugging/profiling support.")
if(DEBUG_QML)
    message(STATUS "To enable the QML debugger/profiler, run with: '-qmljsdebugger=port:1234'")
    add_compile_definitions(QT_QML_DEBUG)
endif()

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060600 QT_DEBUG_FIND_PACKAGE=ON)

include(Qt6QGCConfiguration)
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Qt version: ${QT_VERSION}")
message(STATUS "Qt spec: ${QT_MKSPEC}")

#######################################################
#               QGroundControl Info
#######################################################

include(CompileOptions)
add_compile_definitions(
    QGC_APPLICATION_NAME="QGroundControl"
    QGC_ORG_NAME="QGroundControl.org"
    QGC_ORG_DOMAIN="org.qgroundcontrol"
)

include(Git)
message(STATUS "QGroundControl version: ${APP_VERSION_STR}")

#######################################################
#                QGroundControl Resources
#######################################################

add_subdirectory(libs)
add_subdirectory(src)

set(QGC_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/qgcimages.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qgcresources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qgroundcontrol.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/InstrumentValueIcons/InstrumentValueIcons.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FirmwarePlugin/APM/APMResources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FirmwarePlugin/PX4/PX4Resources.qrc
)

if(CONFIG_UTM_ADAPTER)
    list(APPEND QGC_RESOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UTMSP/utmsp.qrc
    )
else()
    list(APPEND QGC_RESOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UTMSP/dummy/utmsp_dummy.qrc
    )
endif()

if(BUILD_TESTING)
    list(APPEND QGC_RESOURCES
        UnitTest.qrc
    )
endif()

#######################################################
#               QGroundControl Target
#######################################################

qt_add_executable(${PROJECT_NAME} ${QGC_RESOURCES})

if(ANDROID)
    add_subdirectory(android)
elseif(APPLE)
    qt_add_resources(${PROJECT_NAME} "Resources"
        FILES
            ${CMAKE_SOURCE_DIR}/resources/icons/macx.icns
    )
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            MACOSX_BUNDLE TRUE
    )
elseif(WIN32)
    qt_add_resources(${PROJECT_NAME} "Resources"
        FILES
            ${CMAKE_SOURCE_DIR}/windows/QGroundControl.rc
    )
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            WIN32_EXECUTABLE TRUE
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE qgc)

#######################################################
#		Install Configuration
#######################################################

install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/resources/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/qgroundcontrol
)
install(
    FILES ${CMAKE_SOURCE_DIR}/deploy/org.mavlink.qgroundcontrol.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(
    FILES ${CMAKE_SOURCE_DIR}/resources/icons/qgroundcontrol.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps/
    RENAME org.mavlink.qgroundcontrol.png
)
configure_file(
    ${CMAKE_SOURCE_DIR}/deploy/org.mavlink.qgroundcontrol.metainfo.xml.in
    ${CMAKE_BINARY_DIR}/metainfo/org.mavlink.qgroundcontrol.metainfo.xml
    @ONLY
)
install(
    FILES ${CMAKE_BINARY_DIR}/metainfo/org.mavlink.qgroundcontrol.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo/
)

if(NOT ANDROID)
    qt_generate_deploy_qml_app_script(TARGET ${PROJECT_NAME} OUTPUT_SCRIPT deploy_script)
    install(SCRIPT ${deploy_script})
endif()
