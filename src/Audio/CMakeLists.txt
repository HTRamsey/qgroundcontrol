find_package(Qt6 REQUIRED COMPONENTS Core Qml TextToSpeech)

qt_add_library(Audio STATIC
	AudioOutput.cc
	AudioOutput.h
)

target_link_libraries(Audio
	PRIVATE
		Utilities
	PUBLIC
		Qt6::Core
		Qt6::QmlIntegration
		Qt6::TextToSpeech
)

if(LINUX)
	find_program(flite)
	if(flite_FOUND)
		message(STATUS "Audio: Linking flite")
		target_link_libraries(Audio PUBLIC flite)
	endif()
	find_program(speechd)
	if(speechd_FOUND)
		message(STATUS "Audio: Linking speechd")
		target_link_libraries(Audio PUBLIC speechd)
	endif()
endif()


target_include_directories(Audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
