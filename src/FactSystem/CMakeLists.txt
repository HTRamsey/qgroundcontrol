add_subdirectory(FactControls)

find_package(Qt6 REQUIRED COMPONENTS Core Qml)

target_sources(QGC
    PRIVATE
        Fact.cc
        Fact.h
        FactGroup.cc
        FactGroup.h
        FactMetaData.cc
        FactMetaData.h
        FactValueSliderListModel.cc
        FactValueSliderListModel.h
        ParameterManager.cc
        ParameterManager.h
        SettingsFact.cc
        SettingsFact.h
)

target_link_libraries(QGC
    PRIVATE
        Qt6::Core
        Qt6::Qml
)

target_include_directories(QGC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
