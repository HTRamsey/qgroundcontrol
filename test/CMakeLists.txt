include(CMakeDependentOption)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    cmake_dependent_option(QGC_BUILD_TESTING "Enable testing" ON "CMAKE_BUILD_TYPE STREQUAL Debug" OFF)
    if(QGC_BUILD_TESTING)

        message("Building tests")
        include(CTest)
        enable_testing()

        qt_add_library(qgctest
            STATIC
                UnitTestList.cc
                UnitTest.qrc
        )

        target_compile_definitions(qgctest PUBLIC UNITTEST_BUILD) # TODO: QGC_UNITTEST_BUILD

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

        add_subdirectory(AnalyzeView)
        add_subdirectory(Audio)
        add_subdirectory(FactSystem)
        add_subdirectory(Geo)
        add_subdirectory(MissionManager)
        add_subdirectory(qgcunittest)
        add_subdirectory(QmlControls)
        add_subdirectory(ui)
        add_subdirectory(Vehicle)

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

        target_link_libraries(qgctest
            PUBLIC
                AnalyzeViewTest
                AudioTest
                FactSystemTest
                GeoTest
                MissionManagerTest
                qgcunittest
                QmlControlsTest
                uiTest
                VehicleTest
        )

        target_link_libraries(${PROJECT_NAME} PRIVATE qgctest)
    else()
        target_compile_definitions(qgctest QT_NO_DEBUG)
    endif()
endif()
