find_package(Qt6 REQUIRED COMPONENTS Core Test)

target_sources(qgctest
    PRIVATE
        AudioOutputTest.cc
        AudioOutputTest.h
)

target_link_libraries(qgctest
    PRIVATE
        Qt6::Test
)

target_include_directories(qgctest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
