/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick                      2.11
import QtQuick.Controls             2.4
import QtQuick.Dialogs              1.3
import QtQuick.Layouts              1.11

import QGroundControl               1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Controllers   1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0

Item {

    property Fact _camJoystickDZ: QGroundControl.settingsManager.nextVisionSettings.camJoystickDZ
    property Fact _camJoystickGain: QGroundControl.settingsManager.nextVisionSettings.camJoystickGain

    width:                  mainCol.width  + (ScreenTools.defaultFontPixelWidth  * 2)
    height:                 mainCol.height + (ScreenTools.defaultFontPixelHeight * 2)
    readonly property real axisMonitorWidth: ScreenTools.defaultFontPixelWidth * 32
    Column {
        id:                 mainCol
        anchors.centerIn:   parent
        spacing:            ScreenTools.defaultFontPixelHeight

        ListModel {
            id: pitchRollAxleModel

            ListElement {
                text:       qsTr("Hat Button")
            }
            ListElement {
                text:       qsTr("Left Analog Stick")
            }
            ListElement {
                text:       qsTr("Right Analog Stick")
            }
        }

        GridLayout {
            columns:            2
            columnSpacing:      ScreenTools.defaultFontPixelWidth
            rowSpacing:         ScreenTools.defaultFontPixelHeight
            //---------------------------------------------------------------------
            //-- Enable Joystick
            QGCLabel {
                text:               qsTr("Enable joystick input")
                Layout.alignment:   Qt.AlignVCenter
                Layout.minimumWidth: ScreenTools.defaultFontPixelWidth * 36
            }
            QGCCheckBox {
                id:             enabledSwitch
                enabled:        _activeCamJoystick ? true : false
                onClicked:      globals.activeVehicle.joystickCamEnabled = checked
                Component.onCompleted: {
                    checked = globals.activeVehicle.joystickCamEnabled
                }
                Connections {
                    target: globals.activeVehicle
                    onJoystickCamEnabledChanged: {
                        enabledSwitch.checked = globals.activeVehicle.joystickCamEnabled
                    }
                }
                Connections {
                    target: joystickManager
                    onActiveCamJoystickChanged: {
                        if(_activeCamJoystick) {
                            enabledSwitch.checked = Qt.binding(function() { return globals.activeVehicle.joystickCamEnabled })
                        }
                    }
                }
            }
            //---------------------------------------------------------------------
            //-- Joystick Selector
            QGCLabel {
                text:               qsTr("Active Camera joystick:")
                Layout.alignment:   Qt.AlignVCenter
            }
            QGCComboBox {
                id:                 joystickCombo
                width:              ScreenTools.defaultFontPixelWidth * 40
                Layout.alignment:   Qt.AlignVCenter
                model:              joystickManager.joystickNames
                onActivated:        joystickManager.activeCamJoystickName = textAt(index)
                Component.onCompleted: {
                    var index = joystickCombo.find(joystickManager.activeCamJoystickName)
                    if (index === -1) {
                        console.warn(qsTr("Active Camera joystick name not in combo"), joystickManager.activeCamJoystickName)
                    } else {
                        joystickCombo.currentIndex = index
                    }
                }
                Connections {
                    target: joystickManager
                    onAvailableJoysticksChanged: {
                        var index = joystickCombo.find(joystickManager.activeCamJoystickName)
                        if (index >= 0) {
                            joystickCombo.currentIndex = index
                        }
                    }
                }
            }

            //---------------------------------------------------------------------
            //-- Joystick Analog Axle Selector
            QGCLabel {
                text:               qsTr("Camera Pitch & Roll/Yaw Axle:")
                Layout.alignment:   Qt.AlignVCenter
            }

            QGCComboBox {
                id:                 pitchRollAxleCombo
                width:              ScreenTools.defaultFontPixelWidth * 40
                Layout.minimumWidth: ScreenTools.defaultFontPixelWidth * 20
                Layout.alignment:   Qt.AlignVCenter
                model:              pitchRollAxleModel
                Component.onCompleted: {
                    pitchRollAxleCombo.currentIndex = _activeCamJoystick.camPitchRollAxle
                }
                onActivated:        _activeCamJoystick.camPitchRollAxle = index

            }

            QGCLabel {
                text:                   qsTr("Joystick DZ[%]:")
                color:                  "white"
                font.pointSize:         ScreenTools.isMobile ? ScreenTools.mediumFontPointSize : 9
            }

            FactTextField {
                id:                     _joystickDz
                Layout.preferredWidth:  ScreenTools.isMobile ? 90 : 45
                maximumLength:          6
                font.pointSize:         ScreenTools.isMobile ? ScreenTools.mediumFontPointSize : 9
                fact:                   _camJoystickDZ
                showHelp:               false
                onEditingFinished: {
                    if ( _joystickDz.text < 0 || _joystickDz.text > 25 )
                    {
                        mainWindow.showMessageDialog(qsTr("Bad Dead Zone Value"), qsTr("You must enter a value between 0 - 25, Reverting to Default (3)"))
                        _camJoystickDZ.rawValue = 3
                        _joystickDz.text = 3
                    }
                    joystickManager.activeCamJoystick.setCamJoystickDZ(_joystickDz.text)
                }
            }

            QGCLabel {
                text:                   qsTr("Joystick Gain[%]:")
                color:                  "white"
                font.pointSize:         ScreenTools.isMobile ? ScreenTools.mediumFontPointSize : 9
            }

            FactTextField {
                id:                     _joystickGain
                Layout.preferredWidth:  ScreenTools.isMobile ? 90 : 45
                maximumLength:          6
                font.pointSize:         ScreenTools.isMobile ? ScreenTools.mediumFontPointSize : 9
                fact:                   _camJoystickGain
                showHelp:               false
                onEditingFinished: {
                    if ( _joystickGain.text < 10 || _joystickGain.text > 200 )
                    {
                        mainWindow.showMessageDialog(qsTr("Bad Gain Value"), qsTr("You must enter a value between 10 - 200, Reverting to Default (100)"))
                        _camJoystickGain.rawValue = 100
                        _joystickGain.text = 100
                    }
                    joystickManager.activeCamJoystick.setCamJoystickGain(_joystickGain.text)
                }
            }
        }
        Row {
            spacing:                ScreenTools.defaultFontPixelWidth
            //---------------------------------------------------------------------
            //-- Axis Monitors
            Rectangle {
                id:                 axisRect
                color:              Qt.rgba(0,0,0,0)
                border.color:       qgcPal.text
                border.width:       1
                radius:             ScreenTools.defaultFontPixelWidth * 0.5
                width:              axisGrid.width  + (ScreenTools.defaultFontPixelWidth  * 2)
                height:             axisGrid.height + (ScreenTools.defaultFontPixelHeight * 2)
                GridLayout {
                    id:                 axisGrid
                    columns:            2
                    columnSpacing:      ScreenTools.defaultFontPixelWidth
                    rowSpacing:         ScreenTools.defaultFontPixelHeight
                    anchors.centerIn:   parent
                    QGCLabel {
                        text:               qsTr("Roll/Yaw")
                        Layout.minimumWidth: ScreenTools.defaultFontPixelWidth * 12
                    }
                    AxisMonitor {
                        id:                 rollAxis
                        height:             ScreenTools.defaultFontPixelHeight
                        width:              axisMonitorWidth
                        mapped:             controller.rollAxisMapped
                        reversed:           controller.rollAxisReversed
                    }

                    QGCLabel {
                        id:                 pitchLabel
                        width:              _attitudeLabelWidth
                        text:               qsTr("Pitch")
                    }
                    AxisMonitor {
                        id:                 pitchAxis
                        height:             ScreenTools.defaultFontPixelHeight
                        width:              axisMonitorWidth
                        mapped:             controller.pitchAxisMapped
                        reversed:           controller.pitchAxisReversed
                    }

                    Connections {
                        target:             _activeCamJoystick
                        onManualControlCamQml: {
                            rollAxis.axisValue      = roll_yaw
                            pitchAxis.axisValue     = pitch
                        }
                    }                   
                }
            }
            Rectangle {
                color:              Qt.rgba(0,0,0,0)
                border.color:       qgcPal.text
                border.width:       1
                radius:             ScreenTools.defaultFontPixelWidth * 0.5
                width:              axisRect.width
                height:             axisRect.height
                Flow {
                    width:              ScreenTools.defaultFontPixelWidth * 30
                    spacing:            -1
                    anchors.centerIn:   parent
                    Connections {
                        target:     _activeCamJoystick
                        onRawButtonPressedChanged: {
                            if (buttonMonitorRepeater.itemAt(index)) {
                                buttonMonitorRepeater.itemAt(index).pressed = pressed
                            }
                        }
                    }
                    Repeater {
                        id:         buttonMonitorRepeater
                        model:      _activeCamJoystick ? _activeCamJoystick.totalButtonCount : []
                        Rectangle {
                            width:          ScreenTools.defaultFontPixelHeight * 1.5
                            height:         width
                            border.width:   1
                            border.color:   qgcPal.text
                            color:          pressed ? qgcPal.buttonHighlight : qgcPal.windowShade
                            property bool pressed
                            QGCLabel {
                                anchors.fill:           parent
                                color:                  pressed ? qgcPal.buttonHighlightText : qgcPal.buttonText
                                horizontalAlignment:    Text.AlignHCenter
                                verticalAlignment:      Text.AlignVCenter
                                text:                   modelData
                            }
                        }
                    }
                }
            }
        }
    }
}


