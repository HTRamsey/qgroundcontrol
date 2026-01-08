# ============================================================================
# Utilities Module
# Core utilities, logging, file system helpers, and platform abstractions
# ============================================================================

# ----------------------------------------------------------------------------
# Core Utility Sources
# ----------------------------------------------------------------------------
target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        DeviceInfo.cc
        DeviceInfo.h
        JsonHelper.cc
        JsonHelper.h
        Platform.cc
        Platform.h
        QGC.cc
        QGC.h
        QGCCommandLineParser.cc
        QGCCommandLineParser.h
        QGCLogging.cc
        QGCLogging.h
        QGCLoggingCategory.cc
        QGCLoggingCategory.h
        StateMachine.cc
        StateMachine.h
)

# ----------------------------------------------------------------------------
# Platform-Specific Sources
# ----------------------------------------------------------------------------
# Signal handling not available on mobile platforms
if(NOT ANDROID AND NOT IOS)
    target_sources(${CMAKE_PROJECT_NAME}
        PRIVATE
            SignalHandler.cc
            SignalHandler.h
    )
endif()

# iOS-specific screen management
if(IOS)
    target_sources(${CMAKE_PROJECT_NAME}
        PRIVATE
            MobileScreenMgr.mm
            MobileScreenMgr.h
    )
endif()

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ============================================================================
# Utility Subdirectories
# ============================================================================

add_subdirectory(Async)
add_subdirectory(Audio)
add_subdirectory(Compression)
add_subdirectory(Config)
add_subdirectory(Data)
add_subdirectory(FileSystem)
add_subdirectory(Flight)
add_subdirectory(Geo)
add_subdirectory(Logging)
add_subdirectory(Math)
add_subdirectory(Memory)
add_subdirectory(Network)
add_subdirectory(Security)
add_subdirectory(StateMachine)
add_subdirectory(Telemetry)
