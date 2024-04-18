find_package(Qt6 REQUIRED COMPONENTS Core)

cmake_print_variables(QT_ANDROID_APPLICATION_ARGUMENTS QT_HOST_PATH)

# QT_USE_TARGET_ANDROID_BUILD_DIR
list(APPEND QT_ANDROID_MULTI_ABI_FORWARD_VARS QGC_STABLE_BUILD)
list(APPEND QT_ANDROID_MULTI_ABI_FORWARD_VARS QT_HOST_PATH)
cmake_print_variables(QT_ANDROID_MULTI_ABI_FORWARD_VARS)
# QT_ANDROID_DEPLOY_RELEASE
# set(QT_ANDROID_DEPLOYMENT_TYPE ON CACHE BOOL "Deployment Type")
cmake_dependent_option(QT_ANDROID_SIGN_APK "Enable Signing APK" ON "CMAKE_BUILD_TYPE STREQUAL Release" OFF)
if(QT_ANDROID_SIGN_APK)
    message(STATUS "Signing APK")
    message(STATUS "QT_ANDROID_KEYSTORE_PATH $ENV{QT_ANDROID_KEYSTORE_PATH}")
    message(STATUS "QT_ANDROID_KEYSTORE_ALIAS $ENV{QT_ANDROID_KEYSTORE_ALIAS}")
    # QT_ANDROID_KEYSTORE_STORE_PASS, QT_ANDROID_KEYSTORE_KEY_PASS
endif()

if(NOT Qt6LinguistTools_DIR)
    set(Qt6LinguistTools_DIR ${QT_HOST_PATH}/lib/cmake/Qt6LinguistTools)
endif()

FetchContent_Declare(android_openssl
    DOWNLOAD_EXTRACT_TIMESTAMP true
    URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(android_openssl)
include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
add_android_openssl_libraries(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        # QT_ANDROID_ABIS ${ANDROID_ABI}
        QT_ANDROID_MIN_SDK_VERSION 26
        QT_ANDROID_TARGET_SDK_VERSION 34
        QT_ANDROID_PACKAGE_SOURCE_DIR package
        QT_ANDROID_VERSION_NAME "${CMAKE_PROJECT_VERSION}"
        # QT_ANDROID_VERSION_CODE "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
        QT_QML_ROOT_PATH ${CMAKE_SOURCE_DIR}
)
# get_target_property(QGC_ANDROID_DEPLOY_FILE ${PROJECT_NAME} QT_ANDROID_DEPLOYMENT_SETTINGS_FILE)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/qtandroidserialport qtandroidserialport.build)

qt_add_library(qgcandroid STATIC
	AndroidInit.cpp
	AndroidInterface.cc AndroidInterface.h
)

target_link_libraries(qgcandroid
	PRIVATE
		Qt6::CorePrivate
    PUBLIC
    	Qt6::Core
        qtandroidserialport
)

target_include_directories(qgcandroid PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
