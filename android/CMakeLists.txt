if(NOT ANDROID)
	return()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core SerialPort)

qt_add_library(qgcandroid STATIC
	src/AndroidInit.cpp
	src/AndroidInterface.cc
	src/AndroidInterface.h
	src/AndroidSerial.cpp
	src/AndroidSerial.h
)

# if(NOT QGC_NO_SERIAL_LINK)

target_link_libraries(qgcandroid
	PRIVATE
		Qt6::CorePrivate
		Qt6::SerialPortPrivate
		Joystick
	PUBLIC
		Qt6::Core
		Qt6::SerialPort
)

target_include_directories(qgcandroid PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
