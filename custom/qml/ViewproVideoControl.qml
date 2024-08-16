import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import video
import utils

Pane {
    id: root
    signal snapshot
    signal record

    ColumnLayout {
        enabled: ViewproData.linkUp

        Label {
            text: "Camera Controls"
            font.pointSize: ScreenTools.isAndroid ? ScreenTools.mediumFontPointSize : ScreenTools.largeFontPointSize
            Layout.alignment: Qt.AlignHCenter
            //Layout.fillWidth: true
        }

        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            Button {
                text: "+"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onPressed: Viewpro.zoomInSerial(ViewproCamSettings.zoomSpeed)
                onReleased: Viewpro.focusZoomStopSerial()
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
                onPressed: Viewpro.zoomOutSerial(ViewproCamSettings.zoomSpeed)
                onReleased: Viewpro.focusZoomStopSerial()
                Layout.fillWidth: true
            }
        }

        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight
            visible: !ViewproCamSettings.autoFocus

            Button {
                text: "+"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                onPressed: Viewpro.focusInSerial()
                onReleased: Viewpro.focusZoomStopSerial()
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
                onPressed: Viewpro.focusOutSerial()
                onReleased: Viewpro.focusZoomStopSerial()
                Layout.fillWidth: true
            }
        }

        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            Label {
                text: "PIP Mode:"
            }

            ComboBox {
                Layout.fillWidth: true
                model: ["EO/IR", "IR", "IR/EO", "EO"]

                onActivated: function (currentIndex) {
                    ViewproCamSettings.videoSource = currentIndex
                    switch (currentIndex) {
                        case 0:
                            Viewpro.setVideoSourceEO1IRPIPSerial()
                            break

                        case 1:
                            Viewpro.setVideoSourceIRSerial()
                            break

                        case 2:
                            Viewpro.setVideoSourceIREO1PIPSerial()
                            break

                        case 3:
                            Viewpro.setVideoSourceEO1Serial()
                            break

                        default:
                            break
                    }
                }
            }
        }

        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            Label {
                text: "IR Mode:"
            }

            ComboBox {
                Layout.fillWidth: true
                model: ["White Hot", "Black Hot", "Pseudo Color", "Ext1", "Ext2", "Ext3"]

                onActivated: function (currentIndex) {
                    ViewproCamSettings.irMode = currentIndex
                    switch (currentIndex) {
                        case 0:
                            Viewpro.setIRModeWhiteHotSerial()
                            break

                        case 1:
                            Viewpro.setIRModeBlackHotSerial()
                            break

                        case 2:
                            Viewpro.rainbowIRSerial()
                            break

                        case 3:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                            Viewpro.setIRColorExtSerial(currentIndex + 1 - 3)
                            break

                        default:
                            break
                    }
                }
            }
        }

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
                            Viewpro.gimbalHomeSerial()
                            break

                        case 1:
                            Viewpro.gimbalFollowYawSerial()
                            break

                        case 2:
                            Viewpro.gimbalFollowYawDisableSerial()
                            break

                        case 3:
                            Viewpro.gimbalLookDownSerial()
                            break

                        default:
                            break
                    }
                }
            }
        }

        Button {
            id: trackButton
            Layout.fillWidth: true
            horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
            text: "Track Center"
            checkable: true
            checked: false
            onToggled: checked ? Viewpro.trackingOnSerial() : Viewpro.trackingStopSerial()

            /*Connections {
                target: VideoSettings
                onTrackingSWChanged: trackButton.checked = VideoSettings.trackingSW
            }*/
        }

        RowLayout {
            spacing: ScreenTools.defaultFontPixelHeight
            visible: VideoSettings.recordingType !== VideoSettings.RecordingType.Disabled

            Button {
                Layout.fillWidth: true
                text: VideoSettings.recordingType === VideoSettings.RecordingType.Local ? "Screenshot" : "Picture"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                enabled: ViewproData.linkUp
                onReleased: root.snapshot()
            }

            Button {
                Layout.fillWidth: true
                text: recording ? "Stop" : "Record"
                horizontalPadding: ScreenTools.defaultFontPixelWidth * 2
                enabled: ViewproData.linkUp
                onReleased: { root.record(VideoSettings.recordingFormat, VideoSettings.recordingLocation); recording = !recording; }
                property bool recording: false
            }
        }
    }
}
