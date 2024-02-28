if (ANDROID)
	find_package(Qt6 REQUIRED COMPONENTS Core)

	add_library(qtandroidserialport
		src/qserialport.cpp
		src/qserialport_android.cpp
		src/qserialportinfo.cpp
		src/qserialportinfo_android.cpp
	)

	target_link_libraries(qtandroidserialport
		PUBLIC
			Qt6::Core
			Qt6::CorePrivate
			qgc
	)

	target_include_directories(qtandroidserialport
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/src
	)
endif()
