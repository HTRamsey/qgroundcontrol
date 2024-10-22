import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Pane {
    id: root
    signal snapshot
    signal record

    property Viewpro viewpro  // Reference to your Viewpro class instance

    // Properties for settings
    property int zoomSpeed: 1  // Default zoom speed
    property bool autoFocus: false  // Default autofocus setting
    property bool digitalZoomEO: false
    property bool showOSD: false
    property bool imageFlipEO: false
    property int irDZoom: 1
    property bool imageFlipIR: false
    property int trackingTemplate: 0

    ColumnLayout {
        enabled: true  // Adjust based on your application logic

        Label {
            text: "Camera Controls"
            font.pointSize: ScreenTools.isAndroid ? ScreenTools.mediumFontPointSize : ScreenTools.largeFontPointSize
            Layout.alignment: Qt.AlignHCenter
        }

        // Zoom Controls
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            Button {
                text: "+"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onPressed: viewpro.zoomInSerial(zoomSpeed)
                onReleased: viewpro.focusZoomStopSerial()
                Layout.fillWidth: true
            }

            Label {
                text: "Zoom"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
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

            Button {
                text: "+"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onPressed: viewpro.focusInSerial()
                onReleased: viewpro.focusZoomStopSerial()
                Layout.fillWidth: true
            }

            Label {
                text: "Focus"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
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

            Label {
                text: "PIP Mode:"
            }

            ComboBox {
                Layout.fillWidth: true
                model: ["EO/IR", "IR", "IR/EO", "EO"]

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

            Label {
                text: "IR Mode:"
            }

            ComboBox {
                Layout.fillWidth: true
                model: ["White Hot", "Black Hot", "Pseudo Color", "Ext1", "Ext2", "Ext3"]

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

            Label {
                text: "Gimbal Mode:"
            }

            ComboBox {
                Layout.fillWidth: true
                model: ["Recenter", "Follow Yaw", "Lock Yaw", "Look Down"]

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
        Button {
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

            Label {
                text: "Template Size:"
            }

            ComboBox {
                Layout.fillWidth: true
                model: ["small", "medium", "big", "small-medium", "small-big", "medium-big", "small-medium-big"]

                onActivated: function (currentIndex) {
                    viewpro.trackingTemplateSizeSerial(currentIndex)
                    trackingTemplate = currentIndex
                }
            }
        }

        // EO Stream Settings Label
        Label {
            text: "EO Stream Settings"
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        // Auto Focus
        CheckBox {
            text: "Auto Focus"
            checked: autoFocus
            onCheckedChanged: {
                viewpro.setAutoFocusSerial(checked)
                autoFocus = checked
            }
        }

        // Digital Zoom EO
        CheckBox {
            text: "Digital Zoom"
            checked: digitalZoomEO
            onCheckedChanged: {
                viewpro.setEODigitalZoomSerial(checked)
                digitalZoomEO = checked
            }
        }

        // Show On Screen Display
        CheckBox {
            text: "Show OSD"
            checked: showOSD
            onCheckedChanged: {
                viewpro.setOSD1Serial(checked)
                viewpro.setOSD2Serial(checked)
                showOSD = checked
            }
        }

        // Image Flip EO
        CheckBox {
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

            Label {
                text: "Zoom Speed:"
            }

            ComboBox {
                Layout.fillWidth: true
                model: ["1", "2", "3", "4", "5", "6", "7"]

                onActivated: function (currentIndex) {
                    zoomSpeed = currentIndex + 1
                }
            }
        }

        // IR Stream Settings Label
        Label {
            text: "IR Stream Settings"
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        // IR Digital Zoom
        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            Label {
                text: "IR Digital Zoom:"
            }

            ComboBox {
                Layout.fillWidth: true
                model: ["1x", "2x", "3x", "4x"]

                onActivated: function (currentIndex) {
                    viewpro.zoomToIRSerial(currentIndex + 1)
                    irDZoom = currentIndex + 1
                }
            }
        }

        // Image Flip IR
        CheckBox {
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

            Button {
                Layout.fillWidth: true
                text: "Screenshot"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onReleased: root.snapshot()
            }

            Button {
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
