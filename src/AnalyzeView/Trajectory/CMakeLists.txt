# ============================================================================
# Trajectory Module
# Flight trajectory parsing, export, and visualization
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        CsvTrajectoryParser.cc
        CsvTrajectoryParser.h
        MovingFeaturesDocument.cc
        MovingFeaturesDocument.h
        TimestampedTrajectoryPoint.h
        TrajectoryExportController.cc
        TrajectoryExportController.h
        ULogTrajectoryParser.cc
        ULogTrajectoryParser.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
