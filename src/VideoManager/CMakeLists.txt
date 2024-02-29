find_package(Qt6 REQUIRED COMPONENTS Core Quick Multimedia)

qt_add_library(VideoManager STATIC
    GLVideoItemStub.cc
    GLVideoItemStub.h
    SubtitleWriter.cc
    SubtitleWriter.h
    VideoManager.cc
    VideoManager.h
)

target_link_libraries(VideoManager
    PRIVATE
        Qt6::Multimedia
        api
        Camera
        QmlControls
        Settings
        Vehicle
    PUBLIC
        Qt6::Core
        Qt6::Quick
        qgc
        comm
        FactSystem
        VideoReceiver
)

target_include_directories(VideoManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
