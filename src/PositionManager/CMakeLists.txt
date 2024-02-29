find_package(Qt6 REQUIRED COMPONENTS Core Positioning OPTIONAL_COMPONENTS SerialPort)

qt_add_library(PositionManager STATIC
	PositionManager.cpp
	SimulatedPosition.cc
)

target_link_libraries(PositionManager
	PRIVATE
		Qt6::Positioning
		api
		Vehicle
	PUBLIC
		Qt6::Core
		Qt6::Positioning
		qgc
)

if(NOT ANDROID)
	target_link_libraries(PositionManager
		PRIVATE
			Qt6::SerialPort
	)
endif()

target_include_directories(PositionManager
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
