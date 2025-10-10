qt_add_library(QGCStyleModule STATIC)

qt_add_qml_module(QGCStyleModule
    URI QGroundControl.QGCStyle
    VERSION 1.0
    RESOURCE_PREFIX /qml
    QML_FILES
        QGCButton.qml
        QGCCheckBox.qml
        QGCComboBox.qml
        QGCDelayButton.qml
        QGCFlickable.qml
        QGCGroupBox.qml
        QGCLabel.qml
        QGCListView.qml
        QGCMenu.qml
        QGCMenuItem.qml
        QGCMenuSeparator.qml
        QGCMouseArea.qml
        QGCPageIndicator.qml
        QGCPopupDialog.qml
        QGCRadioButton.qml
        QGCSlider.qml
        QGCSwipeView.qml
        QGCSwitch.qml
        QGCTabBar.qml
        QGCTabButton.qml
        QGCTextField.qml
    IMPORTS
        QtQuick
        QtQuick.Controls
        QtQuick.Layouts
        QtQuick.Dialogs
    NO_PLUGIN
)
