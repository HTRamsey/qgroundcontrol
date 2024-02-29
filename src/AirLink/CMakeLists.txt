option(QGC_AIRLINK_DISABLED "Enable airlink" OFF)
if(NOT ${QGC_AIRLINK_DISABLED})
    find_package(Qt6 COMPONENTS REQUIRED Core Qml Network)

    qt_add_library(AirLink STATIC
        AirlinkLink.cc
        AirlinkLink.h
        AirLinkManager.cc
        AirLinkManager.h
    )

    add_custom_target(AirLinkQml
        SOURCES
            AirLinkSettings.qml
    )

    target_link_libraries(AirLink
        PRIVATE
            Settings
        PUBLIC
            Qt6::Core
            Qt6::Network
            comm
            qgc
    )

    target_include_directories(AirLink PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()
