find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Network OPTIONAL_COMPONENTS SerialPort)

qt_add_library(VehicleSetup STATIC
	Bootloader.cc
	Bootloader.h
	FirmwareImage.cc
	FirmwareImage.h
	FirmwareUpgradeController.cc
	FirmwareUpgradeController.h
	JoystickConfigController.cc
	JoystickConfigController.h
	PX4FirmwareUpgradeThread.cc
	PX4FirmwareUpgradeThread.h
	VehicleComponent.cc
	VehicleComponent.h
)

add_custom_target(VehicleSetupQml
	SOURCES
		FirmwareUpgrade.qml
		JoystickConfigAdvanced.qml
		JoystickConfigButtons.qml
		JoystickConfigCalibration.qml
		JoystickConfigGeneral.qml
		JoystickConfig.qml
		PX4FlowSensor.qml
		SetupParameterEditor.qml
		SetupView.qml
		VehicleSummary.qml
)

target_link_libraries(VehicleSetup
	PRIVATE
		api
		AutoPilotPlugins
		compression
		FirmwarePlugin
		Settings
		Vehicle
	PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Network
		comm
		FactSystem
		Joystick
		qgc
)

if(NOT ANDROID)
	target_link_libraries(VehicleSetup
		PUBLIC
			Qt6::SerialPort
	)
endif()

target_include_directories(VehicleSetup PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
