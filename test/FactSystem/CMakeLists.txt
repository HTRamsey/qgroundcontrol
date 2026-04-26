# ============================================================================
# Fact System Unit Tests
# Tests for fact metadata parsing (Parameter* tests live in test/Vehicle/Parameters)
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        FactGroupTest.cc
        FactGroupTest.h
        FactMetaDataTest.cc
        FactMetaDataTest.h
        PX4ParameterMetaDataTest.cc
        PX4ParameterMetaDataTest.h
        FactSystemTestBase.cc
        FactSystemTestBase.h
        FactSystemTestGeneric.cc
        FactSystemTestGeneric.h
        FactSystemTestPX4.cc
        FactSystemTestPX4.h
        FactTest.cc
        FactTest.h
        FactValueSliderListModelTest.cc
        FactValueSliderListModelTest.h
        ParameterMetaDataTestHelper.h
)

if(NOT QGC_DISABLE_APM_PLUGIN)
    target_sources(${CMAKE_PROJECT_NAME}
        PRIVATE
            APMParameterMetaDataTest.cc
            APMParameterMetaDataTest.h
    )
endif()

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
