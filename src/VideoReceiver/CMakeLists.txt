find_package(Qt6 REQUIRED COMPONENTS Core Quick)

qt_add_library(VideoReceiver STATIC
    VideoReceiver.h
)

target_link_libraries(VideoReceiver
    PUBLIC
        Qt6::Core
)

target_include_directories(VideoReceiver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(EXTRA_LIBRARIES)

if(GST_FOUND)
    target_sources(VideoReceiver
        PRIVATE
            gstqgc.c
            gstqgcvideosinkbin.c
            GStreamer.cc
            GStreamer.h
            GstVideoReceiver.cc
            GstVideoReceiver.h
    )

    target_link_libraries(VideoReceiver
        PUBLIC
            Qt6::Quick
            qgc
            qmlglsink
            Settings
    )
endif()
