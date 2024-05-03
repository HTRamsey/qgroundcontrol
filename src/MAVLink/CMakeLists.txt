add_subdirectory(LibEvents)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)

qt_add_library(MAVLink STATIC
	ImageProtocolManager.cc
	ImageProtocolManager.h
	MAVLinkFTP.cc
	MAVLinkFTP.h
	MAVLinkLib.h
	MAVLinkStreamConfig.cc
	MAVLinkStreamConfig.h
	QGCMAVLink.cc
	QGCMAVLink.h
	SysStatusSensorInfo.cc
	SysStatusSensorInfo.h
)

target_link_libraries(MAVLink
	PRIVATE
		Utilities
	INTERFACE
		libevents_wrapper
	PUBLIC
		Qt6::Core
		Qt6::Gui
		comm
)

target_include_directories(MAVLink PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
