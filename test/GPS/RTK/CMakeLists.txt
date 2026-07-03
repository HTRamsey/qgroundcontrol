# ============================================================================
# RTK Unit Tests
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        GPSFormatterTest.cc
        GPSFormatterTest.h
        GPSEventModelTest.cc
        GPSEventModelTest.h
        RTKSatelliteModelTest.cc
        RTKSatelliteModelTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_qgc_test(GPSFormatterTest LABELS Unit)
add_qgc_test(GPSEventModelTest LABELS Unit)
add_qgc_test(RTKSatelliteModelTest LABELS Unit)

# GPSRtk and SerialGPSTransport need serial support to exercise the live link.
if(NOT QGC_NO_SERIAL_LINK)
    target_sources(${CMAKE_PROJECT_NAME}
        PRIVATE
            GPSRtkTest.cc
            GPSRtkTest.h
            SerialGPSTransportTest.cc
            SerialGPSTransportTest.h
    )
    add_qgc_test(GPSRtkTest LABELS Unit)
    add_qgc_test(SerialGPSTransportTest LABELS Unit)
endif()
