if(ANDROID)
	find_package(Qt6 REQUIRED COMPONENTS Core)

	qt_add_library(qtandroidserialport STATIC
		src/qserialport.cpp
		src/qserialport.h
		src/qserialport_p.h
		src/qserialport_android.cpp
		src/qserialport_android_p.h
		src/qserialportinfo.cpp
		src/qserialportinfo.h
		src/qserialportinfo_p.h
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
