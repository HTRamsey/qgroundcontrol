# ============================================================================
# RTK Module
# GCS-attached RTK base-station provider stack, models, and satellite-signal UI
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        GPSEvent.h
        GPSEventModel.cc
        GPSEventModel.h
        GPSProvider.cc
        GPSProvider.h
        GPSRtk.cc
        GPSRtk.h
        GPSRTKFactGroup.cc
        GPSRTKFactGroup.h
        SatelliteModel.cc
        SatelliteModel.h
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
# RTK QML Module — formatting singleton and satellite-signal QML components.
# ----------------------------------------------------------------------------
qt_add_library(GPSModule STATIC)

target_sources(GPSModule
    PRIVATE
        GPSFormatter.cc
        GPSFormatter.h
)

target_link_libraries(GPSModule PRIVATE Qt6::Core Qt6::QmlIntegration)

set_source_files_properties(GPSSNRThresholds.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(GPSModule
    URI QGroundControl.GPS
    VERSION 1.0
    RESOURCE_PREFIX /qml
    QML_FILES
        FixStatusDot.qml
        GPSSNRThresholds.qml
        GPSStatusBlock.qml
        RTKCorrectionsStatusBlock.qml
        SignalIntegrityBlock.qml
        SignalStrengthBar.qml
    NO_PLUGIN
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE GPSModule)
