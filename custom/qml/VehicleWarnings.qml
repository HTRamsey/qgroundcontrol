import QtQuick

import QGroundControl
import QGroundControl.ScreenTools
import QGroundControl.Controls

Rectangle {
    anchors.margins: -ScreenTools.defaultFontPixelHeight
    height: warningsCol.height
    width: warningsCol.width
    color: Qt.rgba(1, 1, 1, 0.5)
    radius: ScreenTools.defaultFontPixelWidth / 2
    visible: warningsCol.visible

    Column {
        id: warningsCol
        spacing: ScreenTools.defaultFontPixelHeight
        visible: droneWarningsCol.visible || gpuWarningsCol.visible

        Column {
            id: droneWarningsCol
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: ScreenTools.defaultFontPixelHeight
            visible: droneWarningsHeader.visible

            property var _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle

            QGCLabel {
                id: droneWarningsHeader
                anchors.horizontalCenter:   parent.horizontalCenter
                visible:                    vehicleConnectionWarning.visible || gpsLockWarning.visible || prearmWarning.visible
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                font.bold:                  true
                text:                       qsTr("Drone Warnings")
            }

            QGCLabel {
                id: vehicleConnectionWarning
                anchors.horizontalCenter:   parent.horizontalCenter
                visible:                    !_activeVehicle
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                text:                       qsTr("No Connection to Vehicle")
            }

            QGCLabel {
                id: gpsLockWarning
                anchors.horizontalCenter:   parent.horizontalCenter
                visible:                    _activeVehicle && _activeVehicle.requiresGpsFix && !_activeVehicle.coordinate.isValid
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                text:                       qsTr("No GPS Lock for Vehicle")
            }

            QGCLabel {
                id: prearmWarning
                anchors.horizontalCenter:   parent.horizontalCenter
                visible:                    _activeVehicle && !_activeVehicle.armed && _activeVehicle.prearmError && !_activeVehicle.healthAndArmingCheckReport.supported
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                text:                       _activeVehicle ? _activeVehicle.prearmError : ""
            }

            // TODO: Improve this
            QGCLabel {
                id: lowVoltageWarning
                anchors.horizontalCenter:   parent.horizontalCenter
                visible:                    (_activeVehicle && _activeVehicle.batteries.get(0)) ? (_activeVehicle.batteries.get(0).voltage.rawValue < QGroundControl.corePlugin.drone.minTakeoffVoltage) : false
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                text:                       qsTr("Low Voltage")
            }

            // TODO: Improve this
            QGCLabel {
                id: powerLimitWarning
                anchors.horizontalCenter: parent.horizontalCenter
                visible: {
                    if (_activeVehicle) {
                        if (_activeVehicle.vehicleTypeString === "octorotor") {
                            return (_activeVehicle.batteries.get(0).instantPower.rawValue > QGroundControl.corePlugin.drone.liteMaxPower);
                        } else if (_activeVehicle.vehicleTypeString === "quadrotor") {
                            return (_activeVehicle.batteries.get(0).instantPower.rawValue > QGroundControl.corePlugin.drone.heavyMaxPower);
                        }
                    }

                    return false
                }
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                text:                       qsTr("Power Limit")
            }
        }

        Column {
            id: gpuWarningsCol
            spacing: ScreenTools.defaultFontPixelHeight
            anchors.horizontalCenter: parent.horizontalCenter
            visible: gpuWarningsHeader.visible

            QGCLabel {
                id: gpuWarningsHeader
                anchors.horizontalCenter:   parent.horizontalCenter
                visible:                    gpuConnectionWarning.visible || /*highPowerWarning.visible ||*/ psuTempWarning.visible || spoolTempWarning.visible
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                font.bold:                  true
                text:                       qsTr("GPU Warnings")
            }

            QGCLabel {
                id: gpuConnectionWarning
                anchors.horizontalCenter:   parent.horizontalCenter
                visible:                    !QGroundControl.corePlugin.gpu.connected.rawValue
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                text:                       qsTr("No Connection to GPU")
            }

            /*QGCLabel {
                id: highPowerWarning
                anchors.horizontalCenter:   parent.horizontalCenter
                visible:                    _activeVehicle.batteries.get(0).voltage.rawValue < !QGroundControl.corePlugin.gpu.highPowerWarning.rawValue
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                text:                       qsTr("High Power is Off")
            }*/

            QGCLabel {
                id: psuTempWarning
                anchors.horizontalCenter:   parent.horizontalCenter
                visible:                    !QGroundControl.corePlugin.gpu.psuTempWarning
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                text:                       "PSU Temperature " + QGroundControl.corePlugin.gpu.psuTempCritical ? "Critical!" : "Warning"
            }

            QGCLabel {
                id: spoolTempWarning
                anchors.horizontalCenter:   parent.horizontalCenter
                visible:                    !QGroundControl.corePlugin.gpu.spoolTempWarning
                color:                      "black"
                font.pointSize:             ScreenTools.largeFontPointSize
                text:                       "Spool Temperature " + QGroundControl.corePlugin.gpu.spoolTempCritical ? "Critical!" : "Warning"
            }
        }
    }
}
