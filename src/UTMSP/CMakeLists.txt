if(CONFIG_UTM_ADAPTER)
    message(STATUS "UTMSP is Initialized")
    find_package(Qt6 REQUIRED COMPONENTS Core Network Positioning Qml Widgets)
    find_package(Threads REQUIRED)

    qt_add_library(UTMSP STATIC
        UTMSPAircraft.cpp
        UTMSPAircraft.h
        UTMSPAuthorization.cpp
        UTMSPAuthorization.h
        UTMSPBlenderRestInterface.cpp
        UTMSPBlenderRestInterface.h
        UTMSPFlightPlanManager.cpp
        UTMSPFlightPlanManager.h
        UTMSPLogger.h
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

    add_custom_target(UTMSPQml
        SOURCES
            UTMSPAdapterEditor.qml
            UTMSPMapVisuals.qml
            UTMSPActivationStatusBar.qml
    )

    target_include_directories(UTMSP PUBLIC services)

    target_link_libraries(UTMSP
        PRIVATE
            Qt6::Network
            Qt6::Qml
            Qt6::Widgets
            libevents_parser
        PUBLIC
            Qt6::Core
            Qt6::Positioning
            Threads::Threads
            comm
            qgc
    )

    target_include_directories(UTMSP
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/services
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

else()
    # If CONFIG_UTM_ADAPTER is not set, use utmsp_dummy.qrc
    message(STATUS "UTMSP: Dummy is Initialized")

    qt_add_library(UTMSP STATIC
        dummy/utmsp_dummy.qrc
    )

    target_include_directories(UTMSP PUBLIC dummy)
endif()

