import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import QGroundControl
import QGroundControl.ScreenTools
import QGroundControl.Controls
import QGroundControl.Palette

Rectangle {
    id: root
    width: mainLayout.width + (_margins * 2)
    height: mainLayout.height + (_margins * 2)
    color: Qt.rgba(qgcPal.window.r, qgcPal.window.g, qgcPal.window.b, 0.5)
    radius: _margins
    signal snapshot
    signal record

    property real _margins: ScreenTools.defaultFontPixelHeight / 2

    // property Viewpro viewpro  // Reference to your Viewpro class instance

    // Properties for settings
    property int zoomSpeed: 1  // Default zoom speed
    property bool autoFocus: false  // Default autofocus setting
    property bool digitalZoomEO: false
    property bool showOSD: false
    property bool imageFlipEO: false
    property int irDZoom: 1
    property bool imageFlipIR: false
    property int trackingTemplate: 0

    QGCPalette { id: qgcPal; colorGroupEnabled: enabled }

    /*RoundButton {
        id: hideVideoControl
        visible: CameraSettings.showCameraControl
        anchors {
            right: videoControlLoader.right
            top: root.top
        }
        icon.source: "qrc:///qt/qml/comms/res/images/cheveron-outline-right.svg"
        onClicked: {
            show = !show
            icon.source = show ? "qrc:///qt/qml/comms/res/images/cheveron-outline-right.svg" : "qrc:///qt/qml/comms/res/images/video-camera.svg"
        }
        property bool show: true
    }*/

    /*Loader {
        width: Math.min(root.height * 0.25, ScreenTools.defaultFontPixelWidth * 16)
        height: Math.min(root.height * 0.25, ScreenTools.defaultFontPixelWidth * 16)
        anchors {
            margins: ScreenTools.defaultFontPixelWidth * 2
            right: root.right
            bottom: root.bottom
        }
        active: CameraSettings.virtualCamJoystick
        sourceComponent: VirtualJoystick {
            id: camStick
            enabled: (DroneData.viewproEnabled && ViewproData.linkUp) || (DroneData.nextvisionEnabled && NextVisionData.linkUp)

            Timer {
                interval: 250
                running: camStick.enabled
                repeat: true
                onTriggered: {
                    if(DroneData.nextvisionEnabled) {
                        NextVision.sendGimbalVirtualCmd(-camStick.axis.x, camStick.axis.y)
                    } else if(DroneData.viewproEnabled) {
                        Viewpro.gimbalManualRCSerial(camStick.axis.x, camStick.axis.y)
                    }
                }
            }
        }
    }*/

    ColumnLayout {
        id: mainLayout
        enabled: true  // Adjust based on your application logic
        anchors.margins: _margins
        anchors.top: parent.top
        anchors.left: parent.left
        spacing: _margins

        QGCLabel {
            text: "Camera Controls"
            font.pointSize: ScreenTools.isAndroid ? ScreenTools.mediumFontPointSize : ScreenTools.largeFontPointSize
            Layout.alignment: Qt.AlignHCenter
        }

        // Zoom Controls
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            QGCButton {
                text: "+"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onPressed: viewpro.zoomInSerial(zoomSpeed)
                onReleased: viewpro.focusZoomStopSerial()
                Layout.fillWidth: true
            }

            QGCLabel {
                text: "Zoom"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            QGCButton {
                text: "-"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onPressed: viewpro.zoomOutSerial(zoomSpeed)
                onReleased: viewpro.focusZoomStopSerial()
                Layout.fillWidth: true
            }
        }

        // Focus Controls
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight
            visible: !autoFocus

            QGCButton {
                text: "+"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onPressed: viewpro.focusInSerial()
                onReleased: viewpro.focusZoomStopSerial()
                Layout.fillWidth: true
            }

            QGCLabel {
                text: "Focus"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            QGCButton {
                text: "-"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onPressed: viewpro.focusOutSerial()
                onReleased: viewpro.focusZoomStopSerial()
                Layout.fillWidth: true
            }
        }

        // PIP Mode Controls
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            QGCLabel {
                text: "PIP Mode:"
            }

            QGCComboBox {
                Layout.fillWidth: true
                model: ["EO/IR", "IR", "IR/EO", "EO"]
                sizeToContents: true

                onActivated: function (currentIndex) {
                    switch (currentIndex) {
                        case 0:
                            viewpro.setVideoSourceEO1IRPIPSerial()
                            break

                        case 1:
                            viewpro.setVideoSourceIRSerial()
                            break

                        case 2:
                            viewpro.setVideoSourceIREO1PIPSerial()
                            break

                        case 3:
                            viewpro.setVideoSourceEO1Serial()
                            break

                        default:
                            break
                    }
                }
            }
        }

        // IR Mode Controls
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            QGCLabel {
                text: "IR Mode:"
            }

            QGCComboBox {
                Layout.fillWidth: true
                model: ["White Hot", "Black Hot", "Pseudo Color", "Ext1", "Ext2", "Ext3"]
                sizeToContents: true

                onActivated: function (currentIndex) {
                    switch (currentIndex) {
                        case 0:
                            viewpro.setIRModeWhiteHotSerial()
                            break

                        case 1:
                            viewpro.setIRModeBlackHotSerial()
                            break

                        case 2:
                            viewpro.rainbowIRSerial()
                            break

                        case 3:
                        case 4:
                        case 5:
                            viewpro.setIRColorExtSerial(currentIndex - 2)
                            break

                        default:
                            break
                    }
                }
            }
        }

        // Gimbal Mode Controls
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            QGCLabel {
                text: "Gimbal Mode:"
            }

            QGCComboBox {
                Layout.fillWidth: true
                model: ["Recenter", "Follow Yaw", "Lock Yaw", "Look Down"]
                sizeToContents: true

                onActivated: function (currentIndex) {
                    switch (currentIndex) {
                        case 0:
                            viewpro.gimbalHomeSerial()
                            break

                        case 1:
                            viewpro.gimbalFollowYawSerial()
                            break

                        case 2:
                            viewpro.gimbalFollowYawDisableSerial()
                            break

                        case 3:
                            viewpro.gimbalLookDownSerial()
                            break

                        default:
                            break
                    }
                }
            }
        }

        // Tracking Control
        QGCButton {
            id: trackButton
            Layout.fillWidth: true
            horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
            text: "Track Center"
            checkable: true
            checked: false
            onToggled: checked ? viewpro.trackingOnSerial() : viewpro.trackingStopSerial()
        }

        // Tracking Template Size
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            QGCLabel {
                text: "Template Size:"
            }

            QGCComboBox {
                Layout.fillWidth: true
                model: ["small", "medium", "big", "small-medium", "small-big", "medium-big", "small-medium-big"]
                sizeToContents: true

                onActivated: function (currentIndex) {
                    viewpro.trackingTemplateSizeSerial(currentIndex)
                    trackingTemplate = currentIndex
                }
            }
        }

        // EO Stream Settings Label
        QGCLabel {
            text: "EO Stream Settings"
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        // Auto Focus
        QGCCheckBox {
            text: "Auto Focus"
            checked: autoFocus
            onCheckedChanged: {
                viewpro.setAutoFocusSerial(checked)
                autoFocus = checked
            }
        }

        // Digital Zoom EO
        QGCCheckBox {
            text: "Digital Zoom"
            checked: digitalZoomEO
            onCheckedChanged: {
                viewpro.setEODigitalZoomSerial(checked)
                digitalZoomEO = checked
            }
        }

        // Show On Screen Display
        QGCCheckBox {
            text: "Show OSD"
            checked: showOSD
            onCheckedChanged: {
                viewpro.setOSD1Serial(checked)
                viewpro.setOSD2Serial(checked)
                showOSD = checked
            }
        }

        // Image Flip EO
        QGCCheckBox {
            text: "Image Flip EO"
            checked: imageFlipEO
            onCheckedChanged: {
                viewpro.imageFlipEOSerial(checked)
                imageFlipEO = checked
            }
        }

        // Zoom Speed
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            QGCLabel {
                text: "Zoom Speed:"
            }

            QGCComboBox {
                Layout.fillWidth: true
                model: ["1", "2", "3", "4", "5", "6", "7"]
                sizeToContents: true

                onActivated: function (currentIndex) {
                    zoomSpeed = currentIndex + 1
                }
            }
        }

        // IR Stream Settings Label
        QGCLabel {
            text: "IR Stream Settings"
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        // IR Digital Zoom
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            QGCLabel {
                text: "IR Digital Zoom:"
            }

            QGCComboBox {
                Layout.fillWidth: true
                model: ["1x", "2x", "3x", "4x"]
                sizeToContents: true

                onActivated: function (currentIndex) {
                    viewpro.zoomToIRSerial(currentIndex + 1)
                    irDZoom = currentIndex + 1
                }
            }
        }

        // Image Flip IR
        QGCCheckBox {
            text: "Image Flip IR"
            checked: imageFlipIR
            onCheckedChanged: {
                viewpro.imageFlipIRSerial(checked)
                imageFlipIR = checked
            }
        }

        // Recording Controls
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            QGCButton {
                Layout.fillWidth: true
                text: "Screenshot"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onReleased: root.snapshot()
            }

            QGCButton {
                id: recordButton
                Layout.fillWidth: true
                text: recording ? "Stop" : "Record"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onReleased: {
                    root.record()
                    recording = !recording
                }
                property bool recording: false
            }
        }
    }
}
