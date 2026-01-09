# ============================================================================
# Mission Manager Module
# Handles mission planning, geofences, rally points, and survey patterns
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        BlankPlanCreator.cc
        BlankPlanCreator.h
        CameraCalc.cc
        CameraCalc.h
        CameraSection.cc
        CameraSection.h
        CameraSpec.cc
        CameraSpec.h
        ComplexMissionItem.cc
        ComplexMissionItem.h
        CorridorScanComplexItem.cc
        CorridorScanComplexItem.h
        CorridorScanPlanCreator.cc
        CorridorScanPlanCreator.h
        FixedWingLandingComplexItem.cc
        FixedWingLandingComplexItem.h
        GeoFenceController.cc
        GeoFenceController.h
        GeoFenceManager.cc
        GeoFenceManager.h
        LandingComplexItem.cc
        LandingComplexItem.h
        MissionCommandList.cc
        MissionCommandList.h
        MissionCommandTree.cc
        MissionCommandTree.h
        MissionCommandUIInfo.cc
        MissionCommandUIInfo.h
        MissionController.cc
        MissionController.h
        MissionItem.cc
        MissionItem.h
        MissionManager.cc
        MissionManager.h
        MissionSettingsItem.cc
        MissionSettingsItem.h
        PlanCreator.cc
        PlanCreator.h
        PlanElementController.cc
        PlanElementController.h
        PlanManager.cc
        PlanManager.h
        PlanMasterController.cc
        PlanMasterController.h
        PlanShapes/CsvPlanDocument.cc
        PlanShapes/CsvPlanDocument.h
        PlanShapes/CsvPlanExporter.cc
        PlanShapes/CsvPlanExporter.h
        PlanShapes/CsvPlanImporter.cc
        PlanShapes/CsvPlanImporter.h
        PlanShapes/GeoJsonPlanDocument.cc
        PlanShapes/GeoJsonPlanDocument.h
        PlanShapes/GeoJsonPlanExporter.cc
        PlanShapes/GeoJsonPlanExporter.h
        PlanShapes/GeoJsonPlanImporter.cc
        PlanShapes/GeoJsonPlanImporter.h
        PlanShapes/GeoPackagePlanDocument.cc
        PlanShapes/GeoPackagePlanDocument.h
        PlanShapes/GeoPackagePlanExporter.cc
        PlanShapes/GeoPackagePlanExporter.h
        PlanShapes/GeoPackagePlanImporter.cc
        PlanShapes/GeoPackagePlanImporter.h
        PlanShapes/GeoFormatPlanImporter.cc
        PlanShapes/GeoFormatPlanImporter.h
        PlanShapes/PlanDocument.cc
        PlanShapes/PlanDocument.h
        PlanShapes/GpxPlanDocument.cc
        PlanShapes/GpxPlanDocument.h
        PlanShapes/GpxPlanExporter.cc
        PlanShapes/GpxPlanExporter.h
        PlanShapes/GpxPlanImporter.cc
        PlanShapes/GpxPlanImporter.h
        PlanShapes/KMLPlanDomDocument.cc
        PlanShapes/KMLPlanDomDocument.h
        PlanShapes/KmlPlanExporter.cc
        PlanShapes/KmlPlanExporter.h
        PlanShapes/KmlPlanImporter.cc
        PlanShapes/KmlPlanImporter.h
        PlanShapes/KmzPlanExporter.cc
        PlanShapes/KmzPlanExporter.h
        PlanShapes/KmzPlanImporter.cc
        PlanShapes/KmzPlanImporter.h
        PlanShapes/OpenAirPlanImporter.cc
        PlanShapes/OpenAirPlanImporter.h
        PlanShapes/PlanExporter.cc
        PlanShapes/PlanExporter.h
        PlanShapes/PlanImporter.cc
        PlanShapes/PlanImporter.h
        PlanShapes/PlanStatistics.cc
        PlanShapes/PlanStatistics.h
        PlanShapes/ShpPlanDocument.cc
        PlanShapes/ShpPlanDocument.h
        PlanShapes/ShpPlanExporter.cc
        PlanShapes/ShpPlanExporter.h
        PlanShapes/ShpPlanImporter.cc
        PlanShapes/ShpPlanImporter.h
        PlanShapes/WktPlanDocument.cc
        PlanShapes/WktPlanDocument.h
        PlanShapes/WktPlanExporter.cc
        PlanShapes/WktPlanExporter.h
        PlanShapes/WktPlanImporter.cc
        PlanShapes/WktPlanImporter.h
        RallyPoint.cc
        RallyPointController.cc
        RallyPointController.h
        RallyPoint.h
        RallyPointManager.cc
        RallyPointManager.h
        Section.h
        SimpleMissionItem.cc
        SimpleMissionItem.h
        SpeedSection.cc
        SpeedSection.h
        StructureScanComplexItem.cc
        StructureScanComplexItem.h
        StructureScanPlanCreator.cc
        StructureScanPlanCreator.h
        SurveyComplexItem.cc
        SurveyComplexItem.h
        SurveyPlanCreator.cc
        SurveyPlanCreator.h
        TakeoffMissionItem.cc
        TakeoffMissionItem.h
        TransectStyleComplexItem.cc
        TransectStyleComplexItem.h
        VisualMissionItem.cc
        VisualMissionItem.h
        VTOLLandingComplexItem.cc
        VTOLLandingComplexItem.h
)

# ----------------------------------------------------------------------------
# Mission Definition JSON Resources
# ----------------------------------------------------------------------------
file(GLOB_RECURSE JSON_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.json)
qt_add_resources(${CMAKE_PROJECT_NAME} json_mission_manager
    PREFIX "/json"
    FILES ${JSON_FILES}
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/PlanShapes
)

# qt_add_qml_module(QGC
#     URI QGroundControl.MissionManager
#     VERSION 1.0
#     RESOURCES
#       BlankPlanCreator.png
#       CogWheel.svg
#       StructureScanPlanCreator.png
#       SurveyPlanCreator.png
#     QML_FILES
#       MapLineArrow.qml
#       QGCMapCircleVisuals.qml
#       QGCMapPolygonVisuals.qml
#       QGCMapPolylineVisuals.qml
#     OUTPUT_TARGETS MissionManager_targets
#     IMPORT_PATH ${QT_QML_OUTPUT_DIRECTORY}
#     IMPORTS
#       QtQuick
#       QtQuick.Controls
#       QtLocation
#       QtPositioning

#       QGroundControl
#       QGroundControl.ScreenTools
#       QGroundControl.Palette
#       QGroundControl.Controls
#       QGroundControl.FlightMap
# )
