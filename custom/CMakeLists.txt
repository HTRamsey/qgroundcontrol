message(STATUS "Adding Custom Plugin")
find_package(Qt6 REQUIRED COMPONENTS Core)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
    APPEND PROPERTY COMPILE_DEFINITIONS
    QGC_CUSTOM_BUILD
    CUSTOMHEADER="CustomPlugin.h"
    CUSTOMCLASS=CustomPlugin
)

find_package(Python)
if(Python_FOUND)
    execute_process(
        COMMAND ${Python_EXECUTABLE}
        RESULT_VARIABLE PYTHON_RESULT
        OUTPUT_VARIABLE PYTHON_OUTPUT
        ERROR_VARIABLE PYTHON_ERROR
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        INPUT_FILE updateqrc.py
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    cmake_print_variables(RESULT_VARIABLE OUTPUT_VARIABLE ERROR_VARIABLE)
endif()

list(APPEND CUSTOM_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/qgcimages.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qgcresources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qgroundcontrol.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/custom.qrc
)
set(QGC_RESOURCES ${CUSTOM_RESOURCES} PARENT_SCOPE)

qt_add_library(CustomPlugin STATIC
    src/CustomOptions.cc
    src/CustomOptions.h
    src/CustomPlugin.cc
    src/CustomPlugin.h
    src/AutoPilotPlugin/CustomAutoPilotPlugin.cc
    src/AutoPilotPlugin/CustomAutoPilotPlugin.h
    src/Drone/Drone.cc
    src/Drone/Drone.h
    src/FactGroups/DroneFactGroup.cc
    src/FactGroups/DroneFactGroup.h
    src/FactGroups/GPUFactGroup.cc
    src/FactGroups/GPUFactGroup.h
    src/FactGroups/NextVisionFactGroup.cc
    src/FactGroups/NextVisionFactGroup.h
    src/FactGroups/ViewproFactGroup.cc
    src/FactGroups/ViewproFactGroup.h
    src/FirmwarePlugin/CustomFirmwarePlugin.cc
    src/FirmwarePlugin/CustomFirmwarePlugin.h
    src/FirmwarePlugin/CustomFirmwarePluginFactory.cc
    src/FirmwarePlugin/CustomFirmwarePluginFactory.h
)

target_link_libraries(CustomPlugin
    PRIVATE
        FactSystem
        QGC
        Settings
        Vehicle
    PUBLIC
        Qt6::Core
        API
        APMAutoPilotPlugin
        APMFirmwarePlugin
        FirmwarePlugin
        MAVLink
)

target_include_directories(CustomPlugin
    PUBLIC
        src
        src/AutoPilotPlugin
        src/Drone
        src/FactGroups
        src/FirmwarePlugin
)
