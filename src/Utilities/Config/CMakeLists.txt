# ============================================================================
# Config Utilities
# Feature flags and configuration management
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCFeatureFlags.cc
        QGCFeatureFlags.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
