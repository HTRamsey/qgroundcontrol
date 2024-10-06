/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls

import QGroundControl
import QGroundControl.FlightDisplay
import QGroundControl.FlightMap
import QGroundControl.ScreenTools
import QGroundControl.Controls
import QGroundControl.Palette
import QGroundControl.Vehicle
import QGroundControl.Controllers

FlightDisplayViewGStreamer {
    id: _root
    property bool _showGrid: QGroundControl.settingsManager.videoSettings.gridLines.rawValue

    Connections {
        target: QGroundControl.videoManager
        function onImageFileChanged() {
            _root.grabToImage(function(result) {
                if (!result.saveToFile(QGroundControl.videoManager.imageFile)) {
                    console.error('Error capturing video frame');
                }
            });
        }
    }

    Rectangle {
        color:  Qt.rgba(1,1,1,0.5)
        height: parent.height
        width:  1
        x:      parent.width * 0.33
        visible: _showGrid && !QGroundControl.videoManager.fullScreen
    }
    Rectangle {
        color:  Qt.rgba(1,1,1,0.5)
        height: parent.height
        width:  1
        x:      parent.width * 0.66
        visible: _showGrid && !QGroundControl.videoManager.fullScreen
    }
    Rectangle {
        color:  Qt.rgba(1,1,1,0.5)
        width:  parent.width
        height: 1
        y:      parent.height * 0.33
        visible: _showGrid && !QGroundControl.videoManager.fullScreen
    }
    Rectangle {
        color:  Qt.rgba(1,1,1,0.5)
        width:  parent.width
        height: 1
        y:      parent.height * 0.66
        visible: _showGrid && !QGroundControl.videoManager.fullScreen
    }
}
