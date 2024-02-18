#######################################################
#                   Main QGC library
#######################################################
find_package(Qt6 ${QT_VERSION}
    COMPONENTS
        Bluetooth
        Charts
        Concurrent
        Core
        Location
        Multimedia
        Network
        Positioning
        Quick
        QuickControls2
        QuickWidgets
        OpenGL
        Sql
        Svg
        Test
        TextToSpeech
        Widgets
        Xml
    REQUIRED
        Core5Compat
    HINTS
        ${QT_LIBRARY_HINTS}
)

if(NOT QT_MKSPEC MATCHES "winrt")
    find_package(Qt6 ${QT_VERSION}
        REQUIRED
        COMPONENTS
            SerialPort
        HINTS
            ${QT_LIBRARY_HINTS}
    )
endif()

#######################################################
#               Directory Inclusion
#######################################################

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/libevents
    ${CMAKE_SOURCE_DIR}/libs/mavlink/include/mavlink/v2.0
    ${CMAKE_SOURCE_DIR}/libs/mavlink/include/mavlink/v2.0/all
    ${CMAKE_SOURCE_DIR}/libs/mavlink/include/mavlink/v2.0/common
    ${CMAKE_SOURCE_DIR}/libs/shapelib
)

#######################################################
#               Mobile Specific Sources
#######################################################
set(EXTRA_SRC)

if (MOBILE)
    list(APPEND EXTRA_SRC
        MobileScreenMgr.cc
        MobileScreenMgr.h
    )
else()
    list(APPEND EXTRA_SRC
        RunGuard.cc
        RunGuard.h
    )
endif()

#######################################################
#             Source Files
#######################################################
qt_add_library(qgc
    STATIC
    CmdLineOptParser.cc
    CmdLineOptParser.h
    JsonHelper.cc
    JsonHelper.h
    KMLDomDocument.cc
    KMLDomDocument.h
    KMLHelper.cc
    KMLHelper.h
    LogCompressor.cc
    LogCompressor.h
    main.cc
    QGC.cc
    QGC.h
    QGCApplication.cc
    QGCApplication.h
    QGCCachedFileDownload.cc
    QGCCachedFileDownload.h
    QGCComboBox.cc
    QGCComboBox.h
    QGCConfig.h
    QGCFileDownload.cc
    QGCFileDownload.h
    QGCLoggingCategory.cc
    QGCLoggingCategory.h
    QGCMapPalette.cc
    QGCMapPalette.h
    QGCPalette.cc
    QGCPalette.h
    QGCQGeoCoordinate.cc
    QGCQGeoCoordinate.h
    QGCTemporaryFile.cc
    QGCTemporaryFile.h
    QGCToolbox.cc
    QGCToolbox.h
    ShapeFileHelper.cc
    ShapeFileHelper.h
    SHPFileHelper.cc
    SHPFileHelper.h
    TerrainTile.cc
    TerrainTile.h
    ${EXTRA_SRC}
)

set_source_files_properties(QGCApplication.cc PROPERTIES COMPILE_DEFINITIONS APP_VERSION_STR="${APP_VERSION_STR}")

add_subdirectory(ADSB)
add_subdirectory(AnalyzeView)
add_subdirectory(api)
add_subdirectory(Audio)
add_subdirectory(AutoPilotPlugins)
add_subdirectory(Camera)
add_subdirectory(comm)
add_subdirectory(Compression)
add_subdirectory(FactSystem)
add_subdirectory(FirmwarePlugin)
add_subdirectory(FlightDisplay)
add_subdirectory(FlightMap)
add_subdirectory(FollowMe)
add_subdirectory(Geo)
add_subdirectory(GPS)
add_subdirectory(Joystick)
add_subdirectory(MissionManager)
add_subdirectory(PlanView)
add_subdirectory(PositionManager)
add_subdirectory(QmlControls)
add_subdirectory(QtLocationPlugin)
add_subdirectory(Settings)
add_subdirectory(Terrain)
add_subdirectory(uas)
add_subdirectory(ui)
add_subdirectory(UTMSP)
add_subdirectory(Vehicle)
add_subdirectory(VehicleSetup)
add_subdirectory(VideoManager)
add_subdirectory(VideoReceiver)
add_subdirectory(Viewer3D)

#######################################################
#               Linking Libraries
#######################################################
target_link_libraries(qgc
    PRIVATE
        shp
        Qt6::QuickControls2
    PUBLIC
        Qt6::QuickWidgets
        Qt6::Widgets
        Qt6::Core5Compat
        Qt6::Test
        ADSB
        AnalyzeView
        api
        Audio
        AutoPilotPlugins
        Camera
        comm
        compression
        FactSystem
        FirmwarePlugin
        FlightMap
        FollowMe
        Geo
        gps
        Joystick
        MissionManager
        PositionManager
        QmlControls
        QtLocationPlugin
        Settings
        Terrain
        uas
        UTMSP
        Vehicle
        VehicleSetup
        VideoManager
        Viewer3D
        z
)

target_include_directories(qgc
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

#######################################################
#             Test Setup
#######################################################
if (BUILD_TESTING)

    add_custom_target(check
            COMMAND ctest --output-on-failure .
            USES_TERMINAL
    )

    function(add_qgc_test test_name)
        add_test(
                NAME ${test_name}
                COMMAND $<TARGET_FILE:QGroundControl> --unittest:${test_name}
        )
        add_dependencies(check QGroundControl)
    endfunction()

    add_subdirectory(qgcunittest)

    add_qgc_test(ComponentInformationCacheTest)
    add_qgc_test(ComponentInformationTranslationTest)
    add_qgc_test(CameraCalcTest)
    add_qgc_test(CameraSectionTest)
    add_qgc_test(CorridorScanComplexItemTest)
    add_qgc_test(FactSystemTestGeneric)
    add_qgc_test(FactSystemTestPX4)
    #add_qgc_test(FileDialogTest)
    #add_qgc_test(FileManagerTest)
    add_qgc_test(FlightGearUnitTest)
    add_qgc_test(GeoTest)
    add_qgc_test(LinkManagerTest)
    add_qgc_test(LogDownloadTest)
    #add_qgc_test(MessageBoxTest)
    add_qgc_test(MissionCommandTreeTest)
    add_qgc_test(MissionControllerTest)
    add_qgc_test(MissionItemTest)
    add_qgc_test(MissionManagerTest)
    add_qgc_test(MissionSettingsTest)
    add_qgc_test(ParameterManagerTest)
    add_qgc_test(PlanMasterControllerTest)
    add_qgc_test(QGCMapPolygonTest)
    add_qgc_test(QGCMapPolylineTest)
    #add_qgc_test(RadioConfigTest)
    add_qgc_test(SendMavCommandTest)
    add_qgc_test(SimpleMissionItemTest)
    add_qgc_test(SpeedSectionTest)
    add_qgc_test(StructureScanComplexItemTest)
    add_qgc_test(SurveyComplexItemTest)
    add_qgc_test(TCPLinkTest)
    add_qgc_test(TransectStyleComplexItemTest)

    target_link_libraries(qgc PUBLIC qgcunittest)

endif ()
