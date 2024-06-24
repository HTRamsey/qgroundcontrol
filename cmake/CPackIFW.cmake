include(CPackIFW)

set(CPACK_GENERATOR "IFW")
set(CPACK_IFW_VERBOSE ON)

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
    set(CPACK_IFW_ROOT "~/Qt/Tools/QtInstallerFramework/4.8")
    set(CPACK_IFW_TARGET_DIRECTORY "@HomeDir@/${QGC_APP_NAME}")
elseif(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    set(CPACK_IFW_ROOT "C:/Qt/Tools/QtInstallerFramework/4.8")
    set(CPACK_IFW_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/deploy/windows/WindowsQGC.ico")
    set(CPACK_IFW_TARGET_DIRECTORY "@HomeDir@\\${QGC_APP_NAME}")
elseif(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    set(CPACK_IFW_ROOT "~/Qt/Tools/QtInstallerFramework/4.8")
    set(CPACK_IFW_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/deploy/mac/macx.icns")
    set(CPACK_IFW_TARGET_DIRECTORY "@ApplicationsDir@/${QGC_APP_NAME}")
endif()

set(CPACK_IFW_PACKAGE_NAME "${QGC_APP_NAME}")
set(CPACK_IFW_PACKAGE_TITLE "${QGC_APP_NAME} Installer")
set(CPACK_IFW_PACKAGE_PUBLISHER "QGC_ORG_NAME")
set(CPACK_IFW_PRODUCT_URL "${CMAKE_PROJECT_HOMEPAGE_URL}")
set(CPACK_IFW_PACKAGE_WIZARD_STYLE "Aero")
set(CPACK_IFW_PACKAGE_LOGO "${CMAKE_SOURCE_DIR}/resources/QGCLogoFull.svg")
set(CPACK_IFW_PACKAGE_WINDOW_ICON "${CMAKE_SOURCE_DIR}/resources/icons/qgroundcontrol.png")
set(CPACK_IFW_PACKAGE_WIZARD_SHOW_PAGE_LIST OFF)

cpack_ifw_configure_component(${QGC_APP_NAME}
    ESSENTIAL FORCED_INSTALLATION
    VERSION ${QGC_APP_VERSION}
    LICENSES "GPL LICENSE" ${CPACK_RESOURCE_FILE_LICENSE}
    SCRIPT "${CMAKE_SOURCE_DIR}/deploy/installer/packages/org.mavlink.qgroundcontrol/meta/dev/installerscript.qs"
)
