find_package(Qt6 REQUIRED COMPONENTS Core Qml)

qt_add_library(VehicleSetup STATIC
	JoystickConfigController.cc
	JoystickConfigController.h
	VehicleComponent.cc
	VehicleComponent.h
)

add_custom_target(VehicleSetupQml
	SOURCES
		JoystickConfig.qml
		JoystickConfigAdvanced.qml
		JoystickConfigButtons.qml
		JoystickConfigCalibration.qml
		JoystickConfigGeneral.qml
		PX4FlowSensor.qml
		SetupParameterEditor.qml
		SetupView.qml
		VehicleSummary.qml
)

target_link_libraries(VehicleSetup
	PRIVATE
		compression
	PUBLIC
		qgc
)

target_include_directories(VehicleSetup PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT QGC_NO_SERIAL_LINK)
	target_sources(VehicleSetup
		PRIVATE
			Bootloader.cc
			Bootloader.h
			FirmwareImage.cc
			FirmwareImage.h
			FirmwareUpgradeController.cc
			FirmwareUpgradeController.h
			PX4FirmwareUpgradeThread.cc
			PX4FirmwareUpgradeThread.h
	)

	target_link_libraries(VehicleSetup PUBLIC comm)

	target_sources(VehicleSetupQml
		PRIVATE
			FirmwareUpgrade.qml
	)
endif()
