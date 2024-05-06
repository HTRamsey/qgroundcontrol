find_package(Qt6 REQUIRED COMPONENTS Core)

qt_add_library(GPS STATIC)

if(NOT QGC_NO_SERIAL_LINK)
	include(FetchContent)
	FetchContent_Declare(gps_drivers
	    GIT_REPOSITORY https://github.com/PX4/PX4-GPSDrivers.git
	    GIT_TAG main
	    GIT_SHALLOW TRUE
	    PATCH_COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/gpsdrivers.patch
        UPDATE_DISCONNECTED 1
	)
	FetchContent_GetProperties(gps_drivers)
	if(NOT gps_drivers_POPULATED)
		FetchContent_Populate(gps_drivers)
		add_subdirectory(${gps_drivers_SOURCE_DIR} ${gps_drivers_BINARY_DIR} EXCLUDE_FROM_ALL)
	endif()

	target_sources(GPS
		PRIVATE
			GPSManager.cc
			GPSManager.h
			GPSProvider.cc
			GPSProvider.h
			RTCMMavlink.cc
			RTCMMavlink.h

			GPSPositionMessage.h
			definitions.h
			satellite_info.h
			sensor_gnss_relative.h
			sensor_gps.h
			${gps_drivers_SOURCE_DIR}/src/ashtech.cpp
			${gps_drivers_SOURCE_DIR}/src/ashtech.h
			${gps_drivers_SOURCE_DIR}/src/gps_helper.cpp
			${gps_drivers_SOURCE_DIR}/src/gps_helper.h
			${gps_drivers_SOURCE_DIR}/src/mtk.cpp
			${gps_drivers_SOURCE_DIR}/src/mtk.h
			${gps_drivers_SOURCE_DIR}/src/rtcm.cpp
			${gps_drivers_SOURCE_DIR}/src/rtcm.h
			${gps_drivers_SOURCE_DIR}/src/sbf.cpp
			${gps_drivers_SOURCE_DIR}/src/sbf.h
			${gps_drivers_SOURCE_DIR}/src/ubx.cpp
			${gps_drivers_SOURCE_DIR}/src/ubx.h
	)

	target_link_libraries(GPS
		PRIVATE
			Settings
			Utilities
			Vehicle
		PUBLIC
			Qt6::Core
			MAVLink
			QGC
	)

	target_include_directories(GPS
		PRIVATE
			${gps_drivers_SOURCE_DIR}/src
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}
	)
endif()
