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

Item {
    id: _root
    width: height * QGroundControl.videoManager.thermalAspectRatio
    height: _camera ? (_camera.thermalMode === MavlinkCameraControl.THERMAL_FULL ? parent.height : (_camera.thermalMode === MavlinkCameraControl.THERMAL_PIP ? ScreenTools.defaultFontPixelHeight * 12 : parent.height * _thermalHeightFactor)) : 0
    anchors.centerIn: parent
    visible: QGroundControl.videoManager.hasThermal && _camera.thermalMode !== MavlinkCameraControl.THERMAL_OFF

    function pipOrNot() {
        if(_camera) {
            if(_camera.thermalMode === MavlinkCameraControl.THERMAL_PIP) {
                anchors.centerIn    = undefined
                anchors.top         = parent.top
                anchors.topMargin   = mainWindow.header.height + (ScreenTools.defaultFontPixelHeight * 0.5)
                anchors.left        = parent.left
                anchors.leftMargin  = ScreenTools.defaultFontPixelWidth * 12
            } else {
                anchors.top         = undefined
                anchors.topMargin   = undefined
                anchors.left        = undefined
                anchors.leftMargin  = undefined
                anchors.centerIn    = parent
            }
        }
    }

    Connections {
        target:                 _camera
        onThermalModeChanged:   _root.pipOrNot()
    }

    onVisibleChanged: {
        _root.pipOrNot()
    }

    QGCVideoBackground {
        objectName: "thermalVideo"
        anchors.fill: parent
        opacity: _camera ? (_camera.thermalMode === MavlinkCameraControl.THERMAL_BLEND ? _camera.thermalOpacity / 100 : 1.0) : 0
    }
}
