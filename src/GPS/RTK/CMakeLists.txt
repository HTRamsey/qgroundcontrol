# ============================================================================
# RTK Module
# GCS-attached RTK base-station provider stack and RTK corrections UI
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        GPSProvider.cc
        GPSProvider.h
        GPSRtk.cc
        GPSRtk.h
        GPSRTKFactGroup.cc
        GPSRTKFactGroup.h
        TcpGPSTransport.cc
        TcpGPSTransport.h
)

# Serial transport is the only serial-dependent piece.
if(NOT QGC_NO_SERIAL_LINK)
    target_sources(${CMAKE_PROJECT_NAME}
        PRIVATE
            SerialGPSTransport.cc
            SerialGPSTransport.h
    )
endif()

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ----------------------------------------------------------------------------
# GPS/RTK Configuration Resources
# ----------------------------------------------------------------------------
qt_add_resources(${CMAKE_PROJECT_NAME} json_gps_drivers
    PREFIX "/json/Vehicle"
    FILES GPSRTKFact.json
)

# ----------------------------------------------------------------------------
# RTK QML Module — RTK corrections status UI.
# ----------------------------------------------------------------------------
qt_add_library(GPSRTKModule STATIC)

qt_add_qml_module(GPSRTKModule
    URI QGroundControl.GPS.RTK
    VERSION 1.0
    RESOURCE_PREFIX /qml
    QML_FILES
        RTKCorrectionsStatusBlock.qml
    NO_PLUGIN
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE GPSRTKModule)
