find_package(Qt6 REQUIRED COMPONENTS Core)

qt_add_library(qgctest STATIC
    FileSystem/QGCFileDownloadTest.cc
    FileSystem/QGCFileDownloadTest.h
)

target_link_libraries(qgctest
    PRIVATE
        Qt6::Test
        QGC
)

target_include_directories(qgctest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} FileSystem)

qt_add_resources(qgctest "UtilitiesTest_res"
    PREFIX "/"
    FILES
        arducopter.apj
)

add_subdirectory(Audio)
add_subdirectory(Compression)
add_subdirectory(Geo)
