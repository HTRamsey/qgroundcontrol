# ============================================================================
# Data Utilities
# Data export, caching, and validation utilities
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCCache.h
        QGCConfigMigrator.cc
        QGCConfigMigrator.h
        QGCCsvExporter.cc
        QGCCsvExporter.h
        QGCFormatter.cc
        QGCFormatter.h
        QGCJsonSchema.cc
        QGCJsonSchema.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
