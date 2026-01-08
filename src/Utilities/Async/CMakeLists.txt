# ============================================================================
# Async Utilities
# Asynchronous task handling, debouncing, and throttling
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCDebouncer.cc
        QGCDebouncer.h
        QGCMessageBus.cc
        QGCMessageBus.h
        QGCRetryQueue.cc
        QGCRetryQueue.h
        QGCScheduler.cc
        QGCScheduler.h
        QGCStateMachine.cc
        QGCStateMachine.h
        QGCTaskRunner.cc
        QGCTaskRunner.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
