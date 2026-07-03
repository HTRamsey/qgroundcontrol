# ============================================================================
# RTK Unit Tests
# ============================================================================

# GPSRtk and SerialGPSTransport need serial support to exercise the live link.
if(NOT QGC_NO_SERIAL_LINK)
    target_sources(${CMAKE_PROJECT_NAME}
        PRIVATE
            GPSRtkTest.cc
            GPSRtkTest.h
            SerialGPSTransportTest.cc
            SerialGPSTransportTest.h
    )
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    add_qgc_test(GPSRtkTest LABELS Unit)
    add_qgc_test(SerialGPSTransportTest LABELS Unit)
endif()
