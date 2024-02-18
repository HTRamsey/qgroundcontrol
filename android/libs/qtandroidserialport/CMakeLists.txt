
if (ANDROID)
    find_package(Qt6 REQUIRED GLOBAL COMPONENTS Core SerialPort)

    qt_add_library(qtandroidserialport
        STATIC
        qserialport.cpp
        qserialport_android.cpp
        qserialportinfo.cpp
        qserialportinfo_android.cpp
    )

    target_link_libraries(qtandroidserialport
        PRIVATE
            Qt6::SerialPortPrivate
        PUBLIC
            Qt6::Core
            Qt6::SerialPort
    )

    target_include_directories(qtandroidserialport INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
