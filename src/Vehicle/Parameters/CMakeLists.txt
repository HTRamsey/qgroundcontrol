# ============================================================================
# Vehicle Parameters Module
# Parameter download, caching, and PARAM_SET / PARAM_REQUEST_READ state machines
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        HashCheckController.cc
        HashCheckController.h
        HashCheckStateMachine.cc
        HashCheckStateMachine.h
        InitialParameterDownloader.cc
        InitialParameterDownloader.h
        OfflineParameterLoader.cc
        OfflineParameterLoader.h
        ParameterCache.cc
        ParameterCache.h
        ParameterCacheDiagnostics.cc
        ParameterCacheDiagnostics.h
        ParameterFileTextIO.cc
        ParameterFileTextIO.h
        ParameterManager.cc
        ParameterManager.h
        ParameterMavlinkCodec.cc
        ParameterMavlinkCodec.h
        ParameterNameRemapper.cc
        ParameterNameRemapper.h
        ParameterPackFile.cc
        ParameterPackFile.h
        ParameterRequestReadStateMachine.cc
        ParameterRequestReadStateMachine.h
        ParameterSetStateMachine.cc
        ParameterSetStateMachine.h
        ParameterTextLine.cc
        ParameterTextLine.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
