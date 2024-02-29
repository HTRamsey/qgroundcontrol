add_subdirectory(MapItems)
add_subdirectory(Widgets)

find_package(Qt6 REQUIRED COMPONENTS Qml)

add_custom_target(FlightMapQml
	FlightMap.qml
	MapScale.qml
	QGCVideoBackground.qml
)
