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

# GCS-attached RTK base-station provider stack.
add_subdirectory(RTK)

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        GPSEvent.h
        GPSEventModel.cc
        GPSEventModel.h
        GPSManager.cc
        GPSManager.h
        SatelliteModel.cc
        SatelliteModel.h
)

# ----------------------------------------------------------------------------
# GPS QML Module — generic GPS/GNSS display components and formatting singleton.
# Works with any GPS data source (vehicle FactGroups, GCS receiver, RTK base).
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
        SignalIntegrityBlock.qml
        SignalStrengthBar.qml
    NO_PLUGIN
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE GPSModule)
