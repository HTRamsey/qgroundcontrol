# ============================================================================
# Logging Utilities Unit Tests
# Tests for log management, disk writing, and remote streaming
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        LogDiskWriterTest.cc
        LogDiskWriterTest.h
        LogEntryTest.cc
        LogEntryTest.h
        LogManagerTest.cc
        LogManagerTest.h
        LogModelTest.cc
        LogModelTest.h
        LogRemoteSinkTest.cc
        LogRemoteSinkTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
