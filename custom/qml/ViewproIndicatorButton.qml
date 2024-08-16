import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import utils

ToolButton {
    id: root
    text: enabled ? ViewproData.zoom + "x" + "\n" + ViewproData.framerate + "fps" : "--.--\n--.--"
    ToolTip.text: "Viewpro"
    enabled: ViewproData.linkUp
    // visible: ViewproData.linkUp
    flat: true
    icon {
        source: "qrc:///qt/qml/comms/res/images/PatternCamera.png"
        color: root.palette.button
        height: ScreenTools.defaultFontPixelHeight * 2
        width: ScreenTools.defaultFontPixelHeight * 2
    }
    onClicked:  {
        let camInfo = camInfoComponent.createObject(root, {})
        if(!!camInfo)
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
                        text: ViewproData.connected ? "True" : "False"
                    }

                    Label {
                        text: "Resolution:"
                    }
                    Label {
                        text: (ViewproData.resH + "x" + ViewproData.resV)
                    }

                    Label {
                        text: "Sensor Size:"
                    }
                    Label {
                        text: (ViewproData.sensorSizeH + "x" + ViewproData.sensorSizeV)
                    }

                    Label {
                        text: "Stream"
                    }
                    Label {
                        text: ViewproData.stream
                    }
                }
            }
        }
    }
}
