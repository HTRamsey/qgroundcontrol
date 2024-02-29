find_package(Qt6 REQUIRED COMPONENTS Core Qml Xml)

qt_add_library(Camera STATIC
	QGCCameraControl.cc
	QGCCameraControl.h
	QGCCameraIO.cc
	QGCCameraIO.h
	QGCCameraManager.cc
	QGCCameraManager.h
)

target_link_libraries(Camera
	PRIVATE
		Qt6::Qml
		Qt6::Xml
		api
		compression
		FirmwarePlugin
		Joystick
		qgc
		QtLocationPlugin
		Settings
		Vehicle
		VideoManager
	PUBLIC
		Qt6::Core
		comm
		FactSystem
		QmlControls
)

target_include_directories(Camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
