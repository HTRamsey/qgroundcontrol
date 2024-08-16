import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import utils

Rectangle {
    id: root
    height: lastRow.y - mainlabel.y + lastRow.height + ((ScreenTools.isAndroid ? 3 : 2) * (ScreenTools.defaultFontPixelHeight / 2))
    width: lastRow.width + (ScreenTools.defaultFontPixelHeight)
    color: "#80000000"
    radius: ScreenTools.defaultFontPixelHeight / 2
    visible: NextVisionData.connected

    Label {
        id: mainlabel
        text: "Camera Controls"
        anchors {
            margins: ScreenTools.isAndroid ? (ScreenTools.defaultFontPixelHeight / 2) * 1.6 : ScreenTools.defaultFontPixelHeight / 2
            horizontalCenter: root.horizontalCenter
            top: root.top
        }
        color: "White"
        font.pointSize: ScreenTools.isAndroid ? ScreenTools.mediumFontPointSize : ScreenTools.largeFontPointSize
    }

    GridLayout {
        id: topGrid
        columns: 1
        anchors {
            horizontalCenter: root.horizontalCenter
            top: mainlabel.bottom
            margins: ScreenTools.defaultFontPixelHeight
        }

        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            Button {
                //showBorder:     true
                text: "+"
                leftPadding: ScreenTools.defaultFontPixelWidth * 2
                rightPadding: leftPadding
                onPressed: NextVision.zoomIn()
                onReleased: NextVision.zoomStop()
            }
            Text {
                text: "Zoom"
                color: "white"
            }
            Button {
                //showBorder:     true
                text: "-"
                leftPadding: ScreenTools.defaultFontPixelWidth * 2
                rightPadding: leftPadding
                onPressed: NextVision.zoomOut()
                onReleased: NextVision.zoomStop()
            }
        }
    }

    GridLayout {
        id: comboGrid
        columns: 2
        anchors {
            horizontalCenter: root.horizontalCenter
            top: topGrid.bottom
            margins: ScreenTools.defaultFontPixelHeight / 2
        }

        Text {
            text: "Gimbal Mode:"
            color: "white"
            Layout.fillWidth: false
            Layout.alignment: Qt.AlignRight
        }

        ComboBox {
            Layout.preferredWidth: ScreenTools.defaultFontPixelWidth * (ScreenTools.isAndroid ? 12 : 15)
            //centeredLabel:  true
            model: ["Stow", "Pilot", "Hold", "Observation", "Grr", "Park"]
            onActivated: {
                switch (currentIndex) {
                case 0:
                    NextVision.setSysModeStow()
                    break

                case 1:
                    NextVision.setSysModePilot()
                    break

                case 2:
                    NextVision.setSysModeHold()
                    break

                case 3:
                    NextVision.setSysModeObs()
                    break

                case 4:
                    NextVision.setSysModeGrr()
                    break

                case 5:
                    NextVision.setSysModePark()
                    break

                default:
                    break
                }
            }
        }

        Text {
            text: "IR Mode:"
            color: "white"
            Layout.fillWidth: false
            Layout.alignment: Qt.AlignRight
            visible: streamMode.currentIndex !== 0
        }

        ComboBox {
            Layout.preferredWidth: ScreenTools.defaultFontPixelWidth * (ScreenTools.isAndroid ? 12 : 15)
            // centeredLabel:  true
            visible: streamMode.currentIndex !== 0
            model: ["White Hot", "Black Hot", "Color Palette", "B/W Palette"]
            onActivated: {
                switch (currentIndex) {
                case 0:
                    NextVision.setIRPolarityWhite()
                    break

                case 1:
                    NextVision.setIRPolarityBlack()
                    break

                case 2:
                    NextVision.setIRColorGrey()
                    break

                case 3:
                    NextVision.setIRColor1()
                    break

                default:
                    console.log("Invalid Index")
                    break
                }
            }
        }

        Text {
            text: "Stream Mode:"
            color: "white"
            Layout.fillWidth: false
            Layout.alignment: Qt.AlignRight
        }

        ComboBox {
            id: streamMode
            Layout.preferredWidth: ScreenTools.defaultFontPixelWidth * (ScreenTools.isAndroid ? 12 : 15)
            // centeredLabel:  true
            model: ["EO", "IR", "PIP EO/IR", "PIP IR/EO", "SBS EO/IR", "SBS IR/EO", "Fusion"]
            onActivated: {
                switch (currentIndex) {
                case 0:
                    NextVision.setSensorDay()
                    break

                case 1:
                    NextVision.setSensorIR()
                    break

                case 2:
                    // NextVision.
                    break

                case 3:
                    // NextVision.
                    break

                case 4:
                    // NextVision.
                    break

                case 5:
                    // NextVision.
                    break

                case 6:
                    // NextVision.
                    break

                default:
                    console.log("Invalid Index")
                    break
                }
            }
        }
    }

    RowLayout {
        id: lastRow
        anchors {
            horizontalCenter: root.horizontalCenter
            top: comboGrid.bottom
            margins: ScreenTools.defaultFontPixelHeight / 2
        }
        spacing: ScreenTools.defaultFontPixelHeight

        // property var videoStreamManager: mainWindow.videoManager
        Button {
            //showBorder:     true
            text: "Picture"
            leftPadding: ScreenTools.defaultFontPixelWidth
            rightPadding: leftPadding
            onReleased: if (!NextVisionData.systemSnapshotStatus) NextVision.takeSnapshotCh0()
            //onReleased: if (lastRow.videoStreamManager.hasVideo) lastRow.videoStreamManager.grabImage()
        }

        Button {
            //showBorder:     true
            text: NextVisionData.systemRecording ? "Stop" : "Record"
            leftPadding: ScreenTools.defaultFontPixelWidth
            rightPadding: leftPadding
            onReleased: {
                if (NextVisionData.systemRecording) {
                    NextVision.setRecStateOffCh0()
                } else {
                    NextVision.setRecStateOnCh0()
                }
            }

            /*onReleased: {
                if (lastRow.videoStreamManager.hasVideo)
                {
                    if (lastRow.videoStreamManager.recording) lastRow.videoStreamManager.stopRecording()
                    else lastRow.videoStreamManager.startRecording()
                }
            }*/
            //enabled: lastRow.videoStreamManager.hasVideo
        }
    }
}
