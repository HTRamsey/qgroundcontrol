find_package(Qt6 REQUIRED COMPONENTS Core TextToSpeech)

target_sources(QGC
    PRIVATE
        AudioOutput.cc
        AudioOutput.h
)

target_link_libraries(QGC
    PRIVATE
        Qt6::Core
        Qt6::TextToSpeech
)

target_include_directories(QGC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
