# ============================================================================
# Memory Utilities
# Memory pooling and allocation optimization
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCMemoryPool.cc
        QGCMemoryPool.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
