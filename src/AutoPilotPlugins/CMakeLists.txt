add_subdirectory(APM)
add_subdirectory(Common)
add_subdirectory(PX4)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_add_library(AutoPilotPlugins STATIC
	APM/APMAirframeComponent.cc
	APM/APMAirframeComponentController.cc
	APM/APMAutoPilotPlugin.cc
	APM/APMCameraComponent.cc
	APM/APMFlightModesComponent.cc
	APM/APMFlightModesComponentController.cc
	APM/APMFollowComponent.cc
	APM/APMFollowComponentController.cc
	APM/APMHeliComponent.cc
	APM/APMLightsComponent.cc
	APM/APMMotorComponent.cc
	APM/APMPowerComponent.cc
	APM/APMRadioComponent.cc
	APM/APMRemoteSupportComponent.cc
	APM/APMSafetyComponent.cc
	APM/APMSensorsComponent.cc
	APM/APMSensorsComponentController.cc
	APM/APMSubFrameComponent.cc
	APM/APMSubMotorComponentController.cc
	APM/APMTuningComponent.cc

	Common/ESP8266Component.cc
	Common/ESP8266ComponentController.cc
	Common/MotorComponent.cc
	Common/RadioComponentController.cc
	Common/SyslinkComponent.cc
	Common/SyslinkComponentController.cc

	Generic/GenericAutoPilotPlugin.cc

	PX4/ActuatorComponent.cc
	PX4/ActuatorComponent.h
	PX4/AirframeComponentAirframes.cc
	PX4/AirframeComponentAirframes.h
	PX4/AirframeComponent.cc
	PX4/AirframeComponentController.cc
	PX4/AirframeComponentController.h
	PX4/AirframeComponent.h
	PX4/CameraComponent.cc
	PX4/CameraComponent.h
	PX4/FlightModesComponent.cc
	PX4/FlightModesComponent.h
	PX4/PowerComponent.cc
	PX4/PowerComponentController.cc
	PX4/PowerComponentController.h
	PX4/PowerComponent.h
	PX4/PX4AirframeLoader.cc
	PX4/PX4AirframeLoader.h
	PX4/PX4AutoPilotPlugin.cc
	PX4/PX4AutoPilotPlugin.h
	PX4/PX4FlightBehavior.cc
	PX4/PX4FlightBehavior.h
	PX4/PX4RadioComponent.cc
	PX4/PX4RadioComponent.h
	PX4/PX4SimpleFlightModesController.cc
	PX4/PX4SimpleFlightModesController.h
	PX4/PX4TuningComponent.cc
	PX4/PX4TuningComponent.h
	PX4/SafetyComponent.cc
	PX4/SafetyComponent.h
	PX4/SensorsComponent.cc
	PX4/SensorsComponentController.cc
	PX4/SensorsComponentController.h
	PX4/SensorsComponent.h

	AutoPilotPlugin.cc
	AutoPilotPlugin.h
)

target_link_libraries(AutoPilotPlugins
	PRIVATE
		FirmwarePlugin
		Actuators
	PUBLIC
		Qt6::Core
		Qt6::Gui
		Qt6::Qml
		Qt6::Quick
		qgc
		FactSystem
		Vehicle
		VehicleSetup
)

target_include_directories(AutoPilotPlugins
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		APM
		Common
		PX4
)
