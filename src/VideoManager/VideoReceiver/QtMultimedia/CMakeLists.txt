find_package(Qt6 REQUIRED COMPONENTS Core Multimedia MultimediaQuickPrivate Quick)

target_sources(QGC
    PRIVATE
        QtMultimediaReceiver.cc
        QtMultimediaReceiver.h
)

target_link_libraries(QGC
    PRIVATE
        Qt6::Core
        Qt6::Multimedia
        Qt6::MultimediaQuickPrivate
        Qt6::Quick
)

target_include_directories(QGC PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(QGC_ENABLE_QT_VIDEOSTREAMING)
    target_compile_definitions(QGC PRIVATE QGC_QT_STREAMING)
endif()

if(NOT QGC_ENABLE_UVC)
    target_compile_definitions(QGC PRIVATE QGC_DISABLE_UVC)
endif()

target_sources(QGC
    PRIVATE
        UVCReceiver.cc
        UVCReceiver.h
)
