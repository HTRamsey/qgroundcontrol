# ============================================================================
# GPS Unit Tests
# Tests for GPS/GNSS positioning and RTK corrections
# ============================================================================

add_subdirectory(RTCM)
add_subdirectory(NTRIP)
add_subdirectory(Driver)
add_subdirectory(RTK)

# Shared helpers plus tests whose subjects live outside src/GPS
# (VehicleGPSFactGroup → src/Vehicle/FactGroups, UdpForwarder → src/Utilities/Network).
target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        GpsTestHelpers.h
        UdpForwarderTest.cc
        UdpForwarderTest.h
        VehicleGPSFactGroupTest.cc
        VehicleGPSFactGroupTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_qgc_test(UdpForwarderTest LABELS Unit)
add_qgc_test(VehicleGPSFactGroupTest LABELS Unit)
