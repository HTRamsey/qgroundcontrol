# ============================================================================
# Parameter Subsystem Tests
# Tests for ParameterManager, ParameterCache, HashCheckController, and
# the pure-function helpers extracted from ParameterManager.
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        HashCheckTest.cc
        HashCheckTest.h
        ParameterCacheTest.cc
        ParameterCacheTest.h
        ParameterManagerTest.cc
        ParameterManagerTest.h
        ParameterMavlinkCodecTest.cc
        ParameterMavlinkCodecTest.h
        ParameterPackFileTest.cc
        ParameterPackFileTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
