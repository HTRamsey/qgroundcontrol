import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.MultiVehicleManager
import QGroundControl.ScreenTools
import QGroundControl.Palette
import QGroundControl.FactSystem
import QGroundControl.FactControls

MapCircle {
    id: geofenceCircle
    visible: DroneData.geofence.isValid
    center: DroneData.geofence.center
    radius: DroneData.geofence.radius
    border.width: 2
    transitions: [
        Transition {
            to: "outOfRange"
            reversible: true
            ColorAnimation {
                duration: 500
            }
        }
    ]
    states: [
        State {
            name: "outOfRange"
            when: !DroneData.geofence.contains(clickMenu.coord)
            PropertyChanges {
                explicit: false
                geofenceCircle {
                    border.color: "red"
                }
            }
        }
    ]
    onCenterChanged: {
        root.center = geofenceCircle.center
        root.zoomLevel = root.maximumZoomLevel
    }
}
