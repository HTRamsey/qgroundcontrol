# ============================================================================
# Async Utilities Unit Tests
# Tests for asynchronous utilities (debouncing, throttling, task running)
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCDebouncerTest.cc
        QGCDebouncerTest.h
        QGCStateMachineTest.cc
        QGCStateMachineTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
