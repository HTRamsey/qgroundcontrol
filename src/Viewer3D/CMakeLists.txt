find_package(Qt6 REQUIRED COMPONENTS Core Gui Positioning Qml Quick3D Xml)

qt_add_library(Viewer3D STATIC
    CityMapGeometry.cc
    CityMapGeometry.h
    earcut.hpp
    OsmParser.cc
    OsmParser.h
	Viewer3DManager.cc
    Viewer3DManager.h
    Viewer3DQmlBackend.cc
    Viewer3DQmlBackend.h
    Viewer3DQmlVariableTypes.h
    Viewer3DUtils.cc
    Viewer3DUtils.h
)

target_link_libraries(Viewer3D
    PRIVATE
        Settings
	PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Positioning
        Qt6::Qml
        Qt6::Quick3D
        Qt6::Xml
        qgc
)

target_include_directories(Viewer3D
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
