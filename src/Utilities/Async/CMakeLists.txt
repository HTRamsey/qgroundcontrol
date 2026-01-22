# ============================================================================
# Async Utilities
# Debouncing, throttling, and retry queue utilities
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCDebouncer.cc
        QGCDebouncer.h
        QGCRetryQueue.cc
        QGCRetryQueue.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
