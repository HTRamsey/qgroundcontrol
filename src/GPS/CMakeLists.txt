# ============================================================================
# GPS Module
# GPS/GNSS positioning and RTK support
# ============================================================================

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(RTCM)
add_subdirectory(NTRIP)

# The RTK stack is transport-agnostic (see GPSTransport). It builds even without
# serial support so a network base station (TcpGPSTransport) works in that config.
add_subdirectory(Driver)

# RTK provider stack, models, and satellite-signal QML components.
add_subdirectory(RTK)

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        GPSManager.cc
        GPSManager.h
)
