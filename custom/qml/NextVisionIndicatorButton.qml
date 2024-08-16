import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import utils

ToolButton {
    id: root
    // text: enabled ? "Detected" : ""
    ToolTip.text: "NextVision"
    enabled: NextVisionData.linkUp
    // visible: NextVisionData.linkUp
    flat: true
    icon {
        source: "qrc:///qt/qml/comms/res/images/PatternCamera.png"
        color: root.palette.button
        height: ScreenTools.defaultFontPixelHeight * 2
        width: ScreenTools.defaultFontPixelHeight * 2
    }
    onClicked:  {
        let camInfo = camInfoComponent.createObject(root, {})
        if(camInfo)
        {
            camInfo.closed.connect(function () {
                camInfo.destroy()
            })
            camInfo.open()
        }
    }

    Component {
        id: camInfoComponent
        Popup {
            y: root.height
            modal: true
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
            Column {
                id: valuesColumn
                spacing: ScreenTools.defaultFontPixelHeight * 0.5
                width: Math.max(camGrid.width, camLabel.width)
                anchors {
                    margins: ScreenTools.defaultFontPixelHeight
                    centerIn: parent
                }

                Label {
                    id: camLabel
                    text: "Camera Status"
                    anchors.horizontalCenter: valuesColumn.horizontalCenter
                    font.pointSize: ScreenTools.mediumFontPointSize
                }

                GridLayout {
                    id: camGrid
                    anchors {
                        margins: ScreenTools.defaultFontPixelHeight
                        horizontalCenter: valuesColumn.horizontalCenter
                    }
                    columns: 2
                    columnSpacing: ScreenTools.defaultFontPixelWidth * 2

                    Label {
                        text: "Connected:"
                    }
                    Label {
                        text: NextVisionData.connected ? "True" : "False"
                    }

                    Label {
                        text: "Stream"
                    }
                    Label {
                        text: NextVisionData.stream
                    }
                }
            }
        }
    }
}
