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

            // PreFlightSoundCheck { id: soundCheck }

            PreFlightCheckButton {
                id: droneConnectionCheck
                name: "Drone Connection"
                telemetryTextFailure: "No Connection to drone."
                telemetryFailure: !_activeVehicle
            }

            PreFlightCheckButton {
                id: gpuConnectionCheck
                name: "Ground Unit Connection"
                telemetryTextFailure: "No Connection to ground unit."
                telemetryFailure: !QGroundControl.corePlugin.gpu.connected.rawValue
            }

            PreFlightSensorsHealthCheck { id: healthCheck }

            PreFlightGPSCheck {
                id: gpsCheck
                failureSatCount: 8
                allowOverrideSatCount: true
            }

            /*PreFlightCheckButton {
                id: homePositionCheck
                name: qsTr("Home Position")
                telemetryFailure: _activeVehicle ? _activeVehicle.homePosition.isValid : true
                telemetryTextFailure: qsTr("Waiting for drone home position.")
            }*/

            PreFlightCheckButton {
                id: preArmCheck
                name: qsTr("Pre-Arm Checks")
                telemetryFailure: _activeVehicle ? _activeVehicle.prearmError : true
                telemetryTextFailure: _activeVehicle.prearmError
            }

            /*PreFlightCheckButton {
                id: readyToFlyCheck
                name: qsTr("Ready To Fly")
                telemetryFailure: globals.activeVehicle ? globals.activeVehicle.readyToFly : true
                telemetryTextFailure: qsTr("Not Ready To Fly.")
            }*/

            PreFlightCheckButton {
                name: qsTr("Confirm")
                manualText: qsTr("Verify Auto Checks Passed?")
                enabled: droneConnectionCheck.passed && /*gpuConnectionCheck.passed &&*/ healthCheck.passed && gpsCheck.passed && preArmCheck.passed
                visible: enabled
            }
        }

        PreFlightCheckGroup {
            id: finalChecks
            name: qsTr("Final Confirmation")

            PreFlightCheckButton {
                id: runMotorTest
                name: qsTr("Run Motor Test")
                manualText: qsTr("Start Motor Test?")
                enabled: _activeVehicle
                onPassedChanged: {
                    if (passed) {
                        _activeVehicle.motorTest(1, 6, 3, true)
                        _activeVehicle.motorTest(2, 6, 3, true)
                        _activeVehicle.motorTest(3, 6, 3, true)
                        _activeVehicle.motorTest(4, 6, 3, true)
                        if (_activeVehicle.vehicleTypeString === "octorotor") {
                            _activeVehicle.motorTest(5, 6, 3, true)
                            _activeVehicle.motorTest(6, 6, 3, true)
                            _activeVehicle.motorTest(7, 6, 3, true)
                            _activeVehicle.motorTest(8, 6, 3, true)
                        }
                    }
                }
            }

            PreFlightCheckButton {
                name: qsTr("Motor Test")
                manualText: qsTr("Verify Motor Test Passed?")
                enabled: runMotorTest.passed
                visible: enabled
            }
        }
    }
}


// PreFlightCheckLabel {
//     id: homePositionCheck
//     name: "Home Position"
//     telemetryFailure: !DroneData.geofence.isValid
//     telemetryTextFailure: "Waiting for drone home position."
// }

// PreFlightCheckGroup {
//     name: qsTr("Last preparations before launch")

//     PreFlightCheckButton {
//         name: qsTr("Motors")
//         manualText: qsTr("Propellers free? Then throttle up gently. Working properly?")
//     }
// }
