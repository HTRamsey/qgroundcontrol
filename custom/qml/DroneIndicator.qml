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

    property var  _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
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
        anchors.fill:   parent
        onClicked:      mainWindow.showIndicatorDrawer(telemDroneInfoPage, control)
    }

    Component {
        id: telemDroneInfoPage

        ToolIndicatorPage {
            showExpand: false

            contentComponent: Component {
                ColumnLayout {
                    spacing: ScreenTools.defaultFontPixelHeight / 2

                    SettingsGroupLayout {
                        heading: "Drone Status"

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.drone.spotlightStatus.shortDescription
                            labelText:  QGroundControl.corePlugin.drone.spotlightStatus.valueString
                            visible:    QGroundControl.corePlugin.drone.spotlightEnabled.rawValue
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.drone.beaconStatus.shortDescription
                            labelText:  QGroundControl.corePlugin.drone.beaconStatus.valueString
                            visible:    QGroundControl.corePlugin.drone.beaconEnabled.rawValue
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.drone.remoteIdStatus.shortDescription
                            labelText:  QGroundControl.corePlugin.drone.remoteIdStatus.valueString
                            visible:    QGroundControl.corePlugin.drone.remoteIdEnabled.rawValue
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.drone.navigationLightStatus.shortDescription
                            labelText:  QGroundControl.corePlugin.drone.navigationLightStatus.valueString
                            visible:    QGroundControl.corePlugin.drone.navigationLightEnabled.rawValue
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.drone.antiCollisionLightStatus.shortDescription
                            labelText:  QGroundControl.corePlugin.drone.antiCollisionLightStatus.valueString
                            visible:    QGroundControl.corePlugin.drone.antiCollisionLightEnabled.rawValue
                        }
                    }
                }
            }
        }
    }
}
