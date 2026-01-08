# ============================================================================
# Network Utilities
# Common networking helpers, URL utilities, and HTTP request configuration
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCJsonRequest.cc
        QGCJsonRequest.h
        QGCNetworkHelper.cc
        QGCNetworkHelper.h
        QGCNetworkManager.cc
        QGCNetworkManager.h
        QGCNetworkRetry.cc
        QGCNetworkRetry.h
        QGCOAuth2.cc
        QGCOAuth2.h
        QGCRateLimiter.cc
        QGCRateLimiter.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
