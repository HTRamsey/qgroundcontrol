include(CPack)
include(InstallRequiredSystemLibraries)

set(CPACK_VERBATIM_VARIABLES YES)

set(INSTALLER_ROOT ${CMAKE_SOURCE_DIR}/deploy/installer)
set(DRIVERS_PACKAGE_ROOT ${INSTALLER_ROOT}/packages/org.mavlink.drivers)
set(QGC_PACKAGE_ROOT ${INSTALLER_ROOT}/packages/org.mavlink.drivers)

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
    # find_program(LINUXDEPLOYQT linuxdeployqt
    #     HINTS
    #         "$ENV{HOME}/dev/linuxdeployqt/build/tools/linuxdeployqt"
    #         "$ENV{HOME}/project/linuxdeployqt/bin"
    # )
    # configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/deploy-qt-linux.cmake.in"
    #                "${CMAKE_BINARY_DIR}/cmake/deploy-qt-linux.cmake" @ONLY)
    # set(CPACK_PRE_BUILD_SCRIPTS ${CMAKE_BINARY_DIR}/cmake/deploy-qt-linux.cmake)
    set(CPACK_PACKAGE_FILE_NAME "${QGC_APP_NAME}-installer-linux")
elseif(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    # find_program(WINDEPLOYQT windeployqt HINTS ${_qt_bin_dir})
    # configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/deploy-qt-windows.cmake.in"
    #                "${CMAKE_BINARY_DIR}/cmake/deploy-qt-windows.cmake" @ONLY)
    # set(CPACK_PRE_BUILD_SCRIPTS ${CMAKE_BINARY_DIR}/cmake/deploy-qt-windows.cmake)
    set(CPACK_PACKAGE_FILE_NAME "${QGC_APP_NAME}-installer-win64")
elseif(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    # find_program(MACDEPLOYQT macdeployqt HINTS ${_qt_bin_dir})
    # configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/deploy-qt-mac.cmake.in"
    #                "${CMAKE_BINARY_DIR}/cmake/deploy-qt-mac.cmake" @ONLY)
    # set(CPACK_PRE_BUILD_SCRIPTS ${CMAKE_BINARY_DIR}/cmake/deploy-qt-mac.cmake)
    set(CPACK_PACKAGE_FILE_NAME "${QGC_APP_NAME}-installer-darwin")
    set(CPACK_BUNDLE_NAME ${QGC_APP_NAME})
    set(CPACK_BUNDLE_ICON "${INSTALLER_ROOT}/config/logos/macx.icns")
endif()

set(CPACK_PACKAGE_INSTALL_DIRECTORY ${QGC_APP_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})
set(CPACK_PACKAGE_ICON "${INSTALLER_ROOT}/config/icons/qgroundcontrol.png")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(CPACK_PACKAGE_EXECUTABLES ${QGC_APP_NAME})
set(CPACK_CREATE_DESKTOP_LINKS ${QGC_APP_NAME})

option(QGC_ONLINE_INSTALLER "Create Online Installer" OFF)
if(QGC_OFFLINE_INSTALLER)
    cpack_add_component(${QGC_APP_NAME} DOWNLOADED)
else()
    cpack_add_component(${QGC_APP_NAME})
endif()
