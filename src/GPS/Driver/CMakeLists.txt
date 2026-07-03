# ============================================================================
# GPS Driver Module
# PX4 GPS driver integration + GNSS message structs
# ============================================================================

qt_add_library(GPSDriverModule STATIC)

target_sources(GPSDriverModule
    PRIVATE
        GPSDriver.cc
        GPSDriver.h
        GPSTransport.h
        GPSType.h
        definitions.h
        satellite_info.h
        sensor_gnss_relative.h
        sensor_gps.h
)

target_include_directories(GPSDriverModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GPSDriverModule
    PUBLIC
        Qt6::Core
    PRIVATE
        QGCLogging
)

# ----------------------------------------------------------------------------
# PX4 GPS Drivers Integration
# ----------------------------------------------------------------------------
CPMAddPackage(
    NAME px4-gpsdrivers
    GITHUB_REPOSITORY PX4/PX4-GPSDrivers
    GIT_TAG cd6f506afda9bd6e8e4645f094dcf5415f1f8be4
    SOURCE_SUBDIR src
)

# NOTE: Using file(GLOB) for external dependency sources
# CONFIGURE_DEPENDS ensures CMake re-runs if files are added/removed
file(GLOB GPS_DRIVERS_SOURCES
    CONFIGURE_DEPENDS
    "${px4-gpsdrivers_SOURCE_DIR}/src/*.c"
    "${px4-gpsdrivers_SOURCE_DIR}/src/*.cpp"
    "${px4-gpsdrivers_SOURCE_DIR}/src/*.h"
)

add_library(px4-gpsdrivers OBJECT ${GPS_DRIVERS_SOURCES})
target_include_directories(px4-gpsdrivers SYSTEM PRIVATE ${px4-gpsdrivers_SOURCE_DIR}/src)
target_compile_definitions(px4-gpsdrivers PRIVATE GPS_DEFINITIONS_HEADER=<${CMAKE_CURRENT_SOURCE_DIR}/definitions.h>)
target_link_libraries(px4-gpsdrivers PRIVATE Qt6::Core)
qgc_disable_dependency_warnings(px4-gpsdrivers)

# px4 sources and the GPS_DEFINITIONS_HEADER contract stay internal to this
# library so they don't leak onto the main application target.
target_sources(GPSDriverModule PRIVATE $<TARGET_OBJECTS:px4-gpsdrivers>)
target_compile_definitions(GPSDriverModule PRIVATE GPS_DEFINITIONS_HEADER=<${CMAKE_CURRENT_SOURCE_DIR}/definitions.h>)
target_include_directories(GPSDriverModule SYSTEM PRIVATE ${px4-gpsdrivers_SOURCE_DIR}/src)

# Exported for the driver-bridge unit test, which compiles px4 headers directly.
set(QGC_PX4_GPSDRIVERS_INCLUDE_DIR ${px4-gpsdrivers_SOURCE_DIR}/src CACHE INTERNAL "px4-gpsdrivers headers")
set(QGC_GPS_DEFINITIONS_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/definitions.h CACHE INTERNAL "GPS_DEFINITIONS_HEADER path")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE GPSDriverModule)
