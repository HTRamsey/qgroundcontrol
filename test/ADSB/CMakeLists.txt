find_package(Qt6 REQUIRED COMPONENTS Core Test)

target_sources(qgctest
    PRIVATE
        ADSBTest.cc
        ADSBTest.h
)

target_link_libraries(qgctest
    PRIVATE
        Qt6::Core
        Qt6::Test
        QGC
)

target_include_directories(qgctest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

qt_add_resources(qgctest "ADSBTest_res"
    PREFIX "/"
    FILES
        ADSB_Simulator.py
)
