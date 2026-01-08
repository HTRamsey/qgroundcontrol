# ============================================================================
# Data Utilities Unit Tests
# Tests for data handling utilities (cache, CSV, JSON schema)
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCCacheTest.cc
        QGCCacheTest.h
        QGCFormatterTest.cc
        QGCFormatterTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
