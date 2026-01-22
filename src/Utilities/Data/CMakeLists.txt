# ============================================================================
# Data Utilities
# Caching and data structure utilities
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCCache.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
