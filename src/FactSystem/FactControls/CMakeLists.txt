find_package(Qt6 REQUIRED COMPONENTS Core Qml)

target_sources(QGC
    PRIVATE
        FactPanelController.cc
        FactPanelController.h
)

target_link_libraries(QGC
    PRIVATE
        Qt6::Core
        Qt6::Qml
)

target_include_directories(QGC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
