find_package(Qt6 REQUIRED COMPONENTS Core Qml)

qt_add_qml_module(qgctest
    URI qmlcontrolstest
    VERSION 1.0
    QML_FILES
        QGCControlDebug.qml
        QmlTest.qml
    IMPORT_PATH ${QT_QML_OUTPUT_DIRECTORY}
)
