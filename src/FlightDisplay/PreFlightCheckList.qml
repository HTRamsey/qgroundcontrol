/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQml.Models
import QtQuick.Layouts

import QGroundControl
import QGroundControl.ScreenTools
import QGroundControl.Controls
import QGroundControl.FlightDisplay


ColumnLayout {
    spacing: 0.8 * ScreenTools.defaultFontPixelWidth

    property real _verticalMargin: ScreenTools.defaultFontPixelHeight / 2

    Loader {
        id:     modelContainer
        source: "qrc:/qml/QGroundControl/FlightDisplay/DefaultChecklist.qml"
    }

    property bool allChecksPassed:  false
    property var  vehicleCopy:      globals.activeVehicle

    onVehicleCopyChanged: {
        if (checkListRepeater.model) {
            checkListRepeater.model.reset()
        }
    }

    onAllChecksPassedChanged: {
        if (allChecksPassed) {
            globals.activeVehicle.checkListState = Vehicle.CheckListPassed
        } else {
            globals.activeVehicle.checkListState = Vehicle.CheckListFailed
        }
    }

    function _handleGroupPassedChanged(index, passed) {
        if (passed) {
            // Collapse current group
            var group = checkListRepeater.itemAt(index)
            group._checked = false
            // Expand next group
            if (index + 1 < checkListRepeater.count) {
                group = checkListRepeater.itemAt(index + 1)
                group.enabled = true
                group._checked = true
            }
        }

        // Walk the list and check if any group is failing
        var allPassed = true
        for (var i=0; i < checkListRepeater.count; i++) {
            if (!checkListRepeater.itemAt(i).passed) {
                allPassed = false
                break
            }
        }
        allChecksPassed = allPassed;
    }

    //-- Pick a checklist model that matches the current airframe type (if any)
    function _updateModel() {
        var vehicle = globals.activeVehicle
        if (!vehicle) {
            vehicle = QGroundControl.multiVehicleManager.offlineEditingVehicle
        }

        if(vehicle.multiRotor) {
            modelContainer.source = "qrc:/qml/QGroundControl/FlightDisplay/MultiRotorChecklist.qml"
        } else if(vehicle.vtol) {
            modelContainer.source = "qrc:/qml/QGroundControl/FlightDisplay/VTOLChecklist.qml"
        } else if(vehicle.rover) {
            modelContainer.source = "qrc:/qml/QGroundControl/FlightDisplay/RoverChecklist.qml"
        } else if(vehicle.sub) {
            modelContainer.source = "qrc:/qml/QGroundControl/FlightDisplay/SubChecklist.qml"
        } else if(vehicle.fixedWing) {
            modelContainer.source = "qrc:/qml/QGroundControl/FlightDisplay/FixedWingChecklist.qml"
        } else {
            modelContainer.source = "qrc:/qml/QGroundControl/FlightDisplay/DefaultChecklist.qml"
        }
        return
    }

    Component.onCompleted: {
        _updateModel()
    }

    onVisibleChanged: {
        if(globals.activeVehicle) {
            if(visible) {
                _updateModel()
            }
        }
    }

    // We delay the updates when a group passes so the user can see all items green for a moment prior to hiding
    Timer {
        id:         delayedGroupPassed
        interval:   750

        property int index

        onTriggered: _handleGroupPassedChanged(index, true /* passed */)
    }

    function groupPassedChanged(index, passed) {
        if (passed) {
            delayedGroupPassed.index = index
            delayedGroupPassed.restart()
        } else {
            _handleGroupPassedChanged(index, passed)
        }
    }

    // Header/title of checklist
    RowLayout {
        Layout.fillWidth:   true
        height:             1.75 * ScreenTools.defaultFontPixelHeight
        spacing:            0

        QGCLabel {
            Layout.fillWidth:   true
            text:               allChecksPassed ? qsTr("(Passed)") : qsTr("In Progress")
            font.pointSize:     ScreenTools.mediumFontPointSize
        }
        QGCButton {
            width:              1.2 * ScreenTools.defaultFontPixelHeight
            height:             1.2 * ScreenTools.defaultFontPixelHeight
            Layout.alignment:   Qt.AlignVCenter
            onClicked:          checkListRepeater.model.reset()

            QGCColoredImage {
                source:         "/qmlimages/MapSyncBlack.svg"
                color:          qgcPal.buttonText
                anchors.fill:   parent
            }
        }
    }

    // All check list items
    Repeater {
        id:     checkListRepeater
        model:  modelContainer.item.model
    }
}
