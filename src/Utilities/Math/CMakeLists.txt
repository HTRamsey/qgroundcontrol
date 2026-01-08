# ============================================================================
# Math Utilities
# Mathematical interpolation and computation utilities
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCInterpolator.cc
        QGCInterpolator.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
