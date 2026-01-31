target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/MultiSignalSpy.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/MultiSignalSpy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MultiSignalSpyTest.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/MultiSignalSpyTest.h
        ${CMAKE_CURRENT_SOURCE_DIR}/UnitTest.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/UnitTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
