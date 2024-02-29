find_package(Qt6 REQUIRED COMPONENTS Core OPTIONAL_COMPONENTS SerialPort)

qt_add_library(gps STATIC
	Drivers/src/ashtech.cpp
	Drivers/src/ashtech.h
	Drivers/src/base_station.h
	Drivers/src/crc.cpp
	Drivers/src/crc.h
	Drivers/src/emlid_reach.cpp
	Drivers/src/emlid_reach.h
	Drivers/src/femtomes.cpp
	Drivers/src/femtomes.h
	Drivers/src/gps_helper.cpp
	Drivers/src/gps_helper.h
	Drivers/src/mtk.cpp
	Drivers/src/mtk.h
	Drivers/src/nmea.cpp
	Drivers/src/nmea.h
	Drivers/src/rtcm.cpp
	Drivers/src/rtcm.h
	Drivers/src/sbf.cpp
	Drivers/src/sbf.h
	Drivers/src/ubx.cpp
	Drivers/src/ubx.h
	Drivers/src/unicore.cpp
	Drivers/src/unicore.h
	RTCM/RTCMMavlink.cc
	RTCM/RTCMMavlink.h
	definitions.h
	GPSManager.cc
	GPSManager.h
	GPSPositionMessage.h
	GPSProvider.cc
	GPSProvider.h
	satellite_info.h
	sensor_gnss_relative.h
	sensor_gps.h
)

target_link_libraries(gps
	PRIVATE
		Settings
		Vehicle
	PUBLIC
		Qt6::Core
		qgc
)

if(NOT ANDROID)
	target_link_libraries(gps
		PRIVATE
			Qt6::SerialPort
	)
endif()

target_include_directories(gps
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/Drivers/src
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/RTCM
)
