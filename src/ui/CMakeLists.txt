add_subdirectory(preferences)
add_subdirectory(toolbar)

find_package(Qt6 REQUIRED COMPONENTS Qml)

add_custom_target(uiQml
	SOURCES
		AppSettings.qml
		ExitWithErrorWindow.qml
		MainRootWindow.qml
)
