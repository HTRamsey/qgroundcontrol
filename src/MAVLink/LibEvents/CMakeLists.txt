find_package(Qt6 REQUIRED COMPONENTS Core)

include(FetchContent)
FetchContent_Declare(libevents
	GIT_REPOSITORY https://github.com/mavlink/libevents.git
	GIT_TAG main
	GIT_SHALLOW TRUE
	SOURCE_SUBDIR libs/cpp
)
FetchContent_MakeAvailable(libevents)

qt_add_library(libevents_wrapper STATIC
	EventHandler.cc
	EventHandler.h
	HealthAndArmingCheckReport.cc
	HealthAndArmingCheckReport.h
	libevents_includes.h
	logging.cpp
)

target_link_libraries(libevents_wrapper
	PRIVATE
		QmlControls
		Utilities
	PUBLIC
		Qt6::Core
		MAVLink
		libevents
)

target_include_directories(libevents_wrapper
	PUBLIC
		${libevents_SOURCE_DIR}/libs/cpp
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}
)
