# ============================================================================
# Network Utilities Unit Tests
# Tests for networking helper functions
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCJsonRequestTest.cc
        QGCJsonRequestTest.h
        QGCNetworkHelperTest.cc
        QGCNetworkHelperTest.h
        QGCNetworkManagerTest.cc
        QGCNetworkManagerTest.h
        QGCNetworkRetryTest.cc
        QGCNetworkRetryTest.h
        QGCOAuth2Test.cc
        QGCOAuth2Test.h
        QGCRateLimiterTest.cc
        QGCRateLimiterTest.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
