find_package(Qt6 REQUIRED COMPONENTS Core)

qt_add_library(UTMSP STATIC)

set(UTMSP_RESOURCES)

option(CONFIG_UTM_ADAPTER "Enable UTM Adapter" OFF) # TODO: Make this QGC_CONFIG_UTM_ADAPTER
if(CONFIG_UTM_ADAPTER)
    message(STATUS "UTMSP is Initialized")

    find_package(Qt6 REQUIRED COMPONENTS Qml Location Widgets)
    find_package(Threads REQUIRED)

    target_sources(UTMSP
        PRIVATE
            UTMSPAircraft.cpp
            UTMSPAircraft.h
            UTMSPAuthorization.cpp
            UTMSPAuthorization.h
            UTMSPBlenderRestInterface.cpp
            UTMSPBlenderRestInterface.h
            UTMSPFlightPlanManager.cpp
            UTMSPFlightPlanManager.h
            UTMSPManager.cpp
            UTMSPManager.h
            UTMSPNetworkRemoteIDManager.cpp
            UTMSPNetworkRemoteIDManager.h
            UTMSPOperator.cpp
            UTMSPOperator.h
            UTMSPRestInterface.cpp
            UTMSPRestInterface.h
            UTMSPServiceController.cpp
            UTMSPServiceController.h
            UTMSPVehicle.cpp
            UTMSPVehicle.h
    )

    target_link_libraries(UTMSP
        PRIVATE
            nlohmann_json
        PUBLIC
            Qt6::Core
            Qt6::Location
            Qt6::Widgets
            Threads::Threads
            qgc
    )

    target_include_directories(UTMSP
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            services
    )

    add_custom_target(UTMSPQml
        SOURCES
            UTMSPActivationStatusBar.qml
            UTMSPAdapterEditor.qml
            UTMSPMapVisuals.qml
    )

    qt_add_resources(UTMSP_RESOURCES utmsp.qrc)
else()
    message(STATUS "UTMSP: Dummy is Initialized")

    qt_add_resources(UTMSP_RESOURCES dummy/utmsp_dummy.qrc)
endif()

target_sources(UTMSP PUBLIC ${UTMSP_RESOURCES})
