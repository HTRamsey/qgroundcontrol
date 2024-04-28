find_package(Qt6 REQUIRED COMPONENTS Core)

qt_add_library(Mavlink STATIC
	MAVLinkFTP.cc
	MAVLinkFTP.h
	MAVLinkMessage.cc
	MAVLinkMessage.h
	MAVLinkProtocol.cc
	MAVLinkProtocol.h
	MAVLinkSystem.cc
	MAVLinkSystem.h
	PX4LogParser.cc
	PX4LogParser.h
	QGCMAVLink.cc
	QGCMAVLink.h
	ULogParser.cc
	ULogParser.h
)

target_link_libraries(Mavlink
	PRIVATE
		Settings
		Vehicle
	PUBLIC
		Qt6::Core
		AnalyzeView
		qgc
		QmlControls
		Utilities
)

# if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
# 	target_compile_options(Mavlink PRIVATE -Wno-address-of-packed-member)
# endif()

target_include_directories(Mavlink
    PUBLIC
    	${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/libs/mavlink/include/mavlink/v2.0
        ${CMAKE_SOURCE_DIR}/libs/mavlink/include/mavlink/v2.0/all
        ${CMAKE_SOURCE_DIR}/libs/mavlink/include/mavlink/v2.0/common
)
