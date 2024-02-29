find_package(Qt6 REQUIRED COMPONENTS Qml)

add_custom_target(FirstRunPromptDialogsQml
	FirstRunPrompt.qml
	OfflineVehicleFirstRunPrompt.qml
	UnitsFirstRunPrompt.qml
)
