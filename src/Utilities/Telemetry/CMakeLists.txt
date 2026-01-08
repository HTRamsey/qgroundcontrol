# ============================================================================
# Telemetry Utilities
# Time-series data buffering and statistics
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCTelemetryBuffer.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
