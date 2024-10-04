import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.MultiVehicleManager
import QGroundControl.ScreenTools
import QGroundControl.Palette

Item {
    id:             control
    anchors.top:    parent.top
    anchors.bottom: parent.bottom
    width:          telemIcon.width * 1.1

    property bool showIndicator: _hasTelemetry

    property var _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property bool _hasTelemetry: _activeVehicle

    QGCColoredImage {
        id:                 telemIcon
        anchors.top:        parent.top
        anchors.bottom:     parent.bottom
        width:              height
        sourceSize.height:  height
        source:             "/res/helicoptericon.svg"
        fillMode:           Image.PreserveAspectFit
        color:              qgcPal.buttonText
    }

    MouseArea {
        anchors.fill: parent
        onClicked: mainWindow.showIndicatorDrawer(telemDroneInfoPage, control)
    }

    Component {
        id: telemDroneInfoPage

        ToolIndicatorPage {
            showExpand: false

            contentComponent: Component {
                ColumnLayout {
                    spacing: ScreenTools.defaultFontPixelHeight / 2

                    SettingsGroupLayout {
                        heading: qsTr("Drone Status")

                        LabelledLabel {
                            label: qsTr("Model")
                            labelText: _activeVehicle.vehicleTypeString
                        }

                        LabelledLabel {
                            label: qsTr("Ready To Fly")
                            labelText: _activeVehicle.readyToFly ? "True" : "False"
                        }

                        LabelledLabel {
                            label: qsTr("Armed")
                            labelText: _activeVehicle.armed ? "True" : "False"
                        }

                        LabelledLabel {
                            label: qsTr("Flight Mode")
                            labelText: _activeVehicle.flightMode
                        }

                        LabelledLabel {
                            label: qsTr("Latitude")
                            labelText: _activeVehicle.latitude.toFixed(7) + "°"
                        }

                        LabelledLabel {
                            label: qsTr("Longitude")
                            labelText: _activeVehicle.longitude.toFixed(7) + "°"
                        }

                        LabelledLabel {
                            label: _activeVehicle.heading.shortDescription
                            labelText: _activeVehicle.heading.valueString + " " + _activeVehicle.heading.units
                        }

                        LabelledLabel {
                            label: _activeVehicle.throttlePct.shortDescription
                            labelText: _activeVehicle.throttlePct.valueString + " " + _activeVehicle.throttlePct.units
                        }

                        LabelledLabel {
                            label: _activeVehicle.groundSpeed.shortDescription
                            labelText: _activeVehicle.groundSpeed.valueString + " " + _activeVehicle.groundSpeed.units
                        }

                        LabelledLabel {
                            label: _activeVehicle.climbRate.shortDescription
                            labelText: _activeVehicle.climbRate.valueString + " " + _activeVehicle.climbRate.units
                        }

                        LabelledLabel {
                            label: _activeVehicle.distanceToGCS.shortDescription
                            labelText: _activeVehicle.distanceToGCS.valueString + " " + _activeVehicle.distanceToGCS.units
                        }

                        LabelledLabel {
                            label: _activeVehicle.flightDistance.shortDescription
                            labelText: _activeVehicle.flightDistance.valueString + " " + _activeVehicle.flightDistance.units
                        }

                        LabelledLabel {
                            label: _activeVehicle.timeToHome.shortDescription
                            labelText: _activeVehicle.timeToHome.valueString + "ms"
                        }

                        LabelledLabel {
                            label: _activeVehicle.hobbs.shortDescription
                            labelText: _activeVehicle.hobbs.valueString + " " + _activeVehicle.hobbs.units
                        }
                    }

                    SettingsGroupLayout {
                        id: payloadsGroup
                        heading: qsTr("Payloads")

                        property var drone: QGroundControl.corePlugin.drone

                        LabelledLabel {
                            label:      payloadsGroup.drone.spotlightStatus.shortDescription
                            labelText:  payloadsGroup.drone.spotlightStatus.valueString + " " + payloadsGroup.drone.spotlightStatus.units
                            visible:    payloadsGroup.drone.spotlightEnabled.rawValue
                        }

                        LabelledLabel {
                            label:      payloadsGroup.drone.beaconStatus.shortDescription
                            labelText:  payloadsGroup.drone.beaconStatus.valueString + " " + payloadsGroup.drone.beaconStatus.units
                            visible:    payloadsGroup.drone.beaconEnabled.rawValue
                        }

                        LabelledLabel {
                            label:      payloadsGroup.drone.remoteIdStatus.shortDescription
                            labelText:  payloadsGroup.drone.remoteIdStatus.valueString + " " + payloadsGroup.drone.remoteIdStatus.units
                            visible:    payloadsGroup.drone.remoteIdEnabled.rawValue
                        }

                        LabelledLabel {
                            label:      payloadsGroup.drone.navigationLightStatus.shortDescription
                            labelText:  payloadsGroup.drone.navigationLightStatus.valueString + " " + payloadsGroup.drone.navigationLightStatus.units
                            visible:    payloadsGroup.drone.navigationLightEnabled.rawValue
                        }

                        LabelledLabel {
                            label:      payloadsGroup.drone.antiCollisionLightStatus.shortDescription
                            labelText:  payloadsGroup.drone.antiCollisionLightStatus.valueString + " " + payloadsGroup.drone.antiCollisionLightStatus.units
                            visible:    payloadsGroup.drone.antiCollisionLightEnabled.rawValue
                        }

                        LabelledLabel {
                            label:      payloadsGroup.drone.antiCollisionLightState.shortDescription
                            labelText:  payloadsGroup.drone.antiCollisionLightState.valueString + " " + payloadsGroup.drone.antiCollisionLightState.units
                            visible:    payloadsGroup.drone.antiCollisionLightEnabled.rawValue
                        }
                    }
                }
            }
        }
    }
}
