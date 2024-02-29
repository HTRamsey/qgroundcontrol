find_package(Qt6 REQUIRED COMPONENTS Core Qml)

qt_add_library(Joystick STATIC
	Joystick.cc
	Joystick.h
	JoystickManager.cc
	JoystickManager.h
	JoystickMavCommand.cc
	JoystickMavCommand.h
)

if(ANDROID)
	target_sources(Joystick
		PRIVATE
			JoystickAndroid.cc
			JoystickAndroid.h
	)

	target_link_libraries(Joystick
		PUBLIC
			Qt6::CorePrivate
	)
else()
	target_sources(Joystick
		PRIVATE
			JoystickSDL.cc
			JoystickSDL.h
	)
endif()

target_link_libraries(Joystick
	PRIVATE
		Qt6::Qml
		AutoPilotPlugins
		FirmwarePlugin
		Vehicle
		VideoManager
	PUBLIC
		Qt6::Core
		qgc
		QmlControls
)

target_include_directories(Joystick PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
	target_link_libraries(Joystick PUBLIC sdl2)
else()
	find_package(SDL2 REQUIRED)
	if(IS_DIRECTORY "${SDL2_INCLUDE_DIRS}")
		include_directories(${SDL2_INCLUDE_DIRS})
		string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
		target_link_libraries(Joystick PRIVATE ${SDL2_LIBRARIES})
	else()
		include_directories(${SDL2_DIR})
		target_link_libraries(Joystick PRIVATE SDL2::SDL2)
	endif()
endif()
