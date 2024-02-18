if(ANDROID)

    FetchContent_Declare(
        android_openssl
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
        #URL_HASH MD5=c97d6ad774fab16be63b0ab40f78d945
    )
    FetchContent_MakeAvailable(android_openssl)
    include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
    add_android_openssl_libraries(${PROJECT_NAME})

    target_sources(${PROJECT_NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/AndroidInit.cpp
    )

    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            QT_ANDROID_ABIS ${ANDROID_ABI}
            # QT_ANDROID_EXTRA_LIBS ""
            QT_ANDROID_MIN_SDK_VERSION 26
            QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/package
            QT_ANDROID_SDK_BUILD_TOOLS_REVISION 34.0.0
            QT_ANDROID_TARGET_SDK_VERSION 33
            QT_ANDROID_VERSION_NAME ${APP_VERSION_STR}
            # QT_ANDROID_VERSION_CODE ${APP_VERSION}
            # QT_QML_ROOT_PATH ${CMAKE_SOURCE_DIR}/ui/qml # ${CMAKE_BINARY_DIR}/qml
            # QT_ANDROID_DEPLOY_RELEASE
            # QT_ANDROID_KEYSTORE_PATH
            # QT_ANDROID_KEYSTORE_ALIAS
            # QT_ANDROID_KEYSTORE_STORE_PASS
            # QT_ANDROID_KEYSTORE_KEY_PASS
            # QT_ANDROID_SIGN_APK
            # QT_ENABLE_VERBOSE_DEPLOYMENT
            # QT_HOST_PATH
            # QT_PATH_ANDROID_ABI_arm64-v8a
    )

    cmake_print_properties(TARGETS ${PROJECT_NAME} PROPERTIES QT_ANDROID_EXTRA_LIBS)

    add_compile_definitions(__mobile__ __android__)

    target_sources(qgc
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/src/AndroidInterface.cc ${CMAKE_CURRENT_SOURCE_DIR}/src/AndroidInterface.h
    )

    target_include_directories(qgc
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    add_subdirectory(libs/qtandroidserialport)

    target_link_libraries(qgc
        PRIVATE
            qtandroidserialport
    )

endif()
