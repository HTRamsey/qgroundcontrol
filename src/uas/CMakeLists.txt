
qt_add_library(uas
	STATIC
	UASMessageHandler.cc
	UASMessageHandler.h
)

target_link_libraries(uas
	PRIVATE
		qgc
)

target_include_directories(uas INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

