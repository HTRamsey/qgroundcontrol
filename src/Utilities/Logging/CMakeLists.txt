# ============================================================================
# Logging Utilities
# Log rotation and file management
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCLogRotator.cc
        QGCLogRotator.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
