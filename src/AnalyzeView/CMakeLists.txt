find_package(Qt6 REQUIRED COMPONENTS Core Charts Qml)

qt_add_library(AnalyzeView STATIC
	ExifParser.cc
	ExifParser.h
	GeoTagController.cc
	GeoTagController.h
	LogDownloadController.cc
	LogDownloadController.h
	MavlinkConsoleController.cc
	MavlinkConsoleController.h
	MAVLinkInspectorController.cc
	MAVLinkInspectorController.h
	PX4LogParser.cc
	PX4LogParser.h
	ULogParser.cc
	ULogParser.h
)

add_custom_target(AnalyzeViewQml
	SOURCES
		AnalyzePage.qml
		AnalyzeView.qml
		GeoTagPage.qml
		LogDownloadPage.qml
		MavlinkConsolePage.qml
		MAVLinkInspectorPage.qml
		VibrationPage.qml
)

target_link_libraries(AnalyzeView
	PRIVATE
		Qt6::Charts
		FactSystem
		QtLocationPlugin
		Settings
		Vehicle
	PUBLIC
		Qt6::Core
		Qt6::Positioning
		qgc
)

target_include_directories(AnalyzeView PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
