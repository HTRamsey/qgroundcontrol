import QtQuick
import QtQuick.Controls
import QtQml.Models

import QGroundControl
import QGroundControl.ScreenTools
import QGroundControl.Controls
import QGroundControl.FlightDisplay
import QGroundControl.Vehicle

Item {
    property var model: listModel
    PreFlightCheckModel {
        id: listModel

        PreFlightCheckGroup {
            name: qsTr("Flight Plan Checks")

            PreFlightCheckButton {
                name: qsTr("Location")
                manualText: qsTr("Verify you are not in restricted airspace.")
            }

            PreFlightCheckButton {
                name: qsTr("Wind & weather")
                manualText: qsTr("OK for your platform?")
            }

            PreFlightCheckButton {
                name: qsTr("Flight area")
                manualText: qsTr("Launch area and path free of obstacles/people?")
            }
        }

        PreFlightCheckGroup {
            name: qsTr("Multirotor Initial Checks")

            PreFlightCheckButton {
                name: qsTr("Hardware")
                manualText: qsTr("Props mounted and secured?")
            }

            PreFlightCheckButton {
                name: qsTr("Payloads")
                manualText: qsTr("All payloads mounted and secured?")
            }

            PreFlightCheckButton {
                name: qsTr("Hardware")
                manualText: qsTr("Tether Cable Secured?")
            }

            PreFlightCheckButton {
                name: qsTr("Power")
                manualText: qsTr("Battery Charged and High Power on?")
            }
        }

        PreFlightCheckGroup {
            id: droneChecks
            name: qsTr("Auto-Diagnostics Checks")

            PreFlightSoundCheck { id: soundCheck }

            PreFlightSensorsHealthCheck { id: healthCheck }

            PreFlightGPSCheck {
                id: gpsCheck
                failureSatCount: 8
                allowOverrideSatCount: true
            }

            PreFlightCheckButton {
                id: homePositionCheck
                name: qsTr("Home Position")
                telemetryFailure: globals.activeVehicle ? globals.activeVehicle.homePosition.isValid : true
                telemetryTextFailure: qsTr("Waiting for drone home position.")
            }

            PreFlightCheckButton {
                id: preArmCheck
                name: qsTr("Pre-Arm Checks")
                telemetryFailure: globals.activeVehicle ? globals.activeVehicle.prearmError : true
                telemetryTextFailure: globals.activeVehicle.prearmError
            }

            PreFlightCheckButton {
                id: readyToFlyCheck
                name: qsTr("Ready To Fly")
                telemetryFailure: globals.activeVehicle ? globals.activeVehicle.readyToFly : true
                telemetryTextFailure: qsTr("Not Ready To Fly.")
            }

            PreFlightCheckButton {
                name: qsTr("Confirm")
                manualText: qsTr("Verify Auto Checks Passed?")
                enabled: soundCheck.passed && healthCheck.passed && gpsCheck.passed && homePositionCheck.passed && preArmCheck.passed && readyToFlyCheck.passed
            }
        }

        PreFlightCheckGroup {
            id: finalChecks
            name: qsTr("Final Confirmation")

            PreFlightCheckButton {
                name: qsTr("Ready?")
                manualText: qsTr("Confirm")
            }
        }
    }
}

// PreFlightCheckLabel {
//     name: "Drone Connection"
//     telemetryTextFailure: "No Connection to drone."
//     telemetryFailure: !DroneData.connected
// }
// PreFlightCheckLabel {
//     id: groundUnitCheck
//     name: "Ground Unit Connection"
//     telemetryTextFailure: "No Connection to ground unit."
//     telemetryFailure: !GPUData.connected
// }
// PreFlightCheckLabel {
//     id: homePositionCheck
//     name: "Home Position"
//     telemetryFailure: !DroneData.geofence.isValid
//     telemetryTextFailure: "Waiting for drone home position."
// }
// PreFlightCheckLabel {
//     id: readyToFlyCheck
//     name: "Ready To Fly"
//     telemetryTextFailure: "Check Pre-arm Failures."
//     telemetryFailure: !DroneData.readyToFly
// }

// PreFlightCheckGroup {
//     name: qsTr("Last preparations before launch")

//     PreFlightCheckButton {
//         name: qsTr("Motors")
//         manualText: qsTr("Propellers free? Then throttle up gently. Working properly?")
//     }
// }
