find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_add_library(api STATIC
	QGCCorePlugin.cc
	QGCCorePlugin.h
	QGCOptions.cc
	QGCOptions.h
	QGCSettings.cc
	QGCSettings.h
	QmlComponentInfo.cc
	QmlComponentInfo.h
)

target_link_libraries(api
	PRIVATE
		Qt6::Quick
		Qt6::Qml
		Camera
		FactSystem
		Settings
		VideoManager
		VideoReceiver
	PUBLIC
		Qt6::Core
		Qt6::Gui
		qgc
		comm
		QmlControls
)

target_include_directories(api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
