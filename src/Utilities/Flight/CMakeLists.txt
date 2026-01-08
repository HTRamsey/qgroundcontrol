# ============================================================================
# Flight Utilities
# Flight recording, waypoint optimization, and geofencing
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCFlightRecorder.cc
        QGCFlightRecorder.h
        QGCGeofence.cc
        QGCGeofence.h
        QGCWaypointOptimizer.cc
        QGCWaypointOptimizer.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
