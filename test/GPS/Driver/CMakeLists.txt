# ============================================================================
# GPS Driver Unit Tests
# ============================================================================

# GPSDriver facade test exercises the px4 bridge — only built when the Driver
# layer is (serial support enabled).
if(NOT QGC_NO_SERIAL_LINK)
    target_sources(${CMAKE_PROJECT_NAME}
        PRIVATE
            GPSDriverTest.cc
            GPSDriverTest.h
    )

    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    add_qgc_test(GPSDriverTest LABELS Unit)

    # GPSDriverTest compiles px4 headers directly; the app target no longer
    # exposes them since GPSDriverModule keeps px4 internal.
    target_include_directories(${CMAKE_PROJECT_NAME} SYSTEM PRIVATE ${QGC_PX4_GPSDRIVERS_INCLUDE_DIR})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE GPS_DEFINITIONS_HEADER=<${QGC_GPS_DEFINITIONS_HEADER}>)
endif()
