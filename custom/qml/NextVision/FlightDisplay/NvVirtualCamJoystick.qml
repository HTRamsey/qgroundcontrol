/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick 2.3

import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Controls      1.0
import QGroundControl.Palette       1.0
import QGroundControl.Vehicle       1.0

Item {
    property var _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    readonly property color     _colorWhite:        "#ffffff"
    property bool _showZoom:             QGroundControl.settingsManager.nextVisionSettings.virtualCamJoystickShowZoom.rawValue

    Timer {
        interval:   80  // 25Hz / 2, same as real joystick rate
        running:    QGroundControl.settingsManager.nextVisionSettings.virtualCamJoystick.value && _activeVehicle
        repeat:     true
        onTriggered: {
            if (_activeVehicle)
            {
                _activeVehicle.virtualCamJoystickValue(camStick.xAxis, camStick.yAxis)
            }
        }
    }

    QGCColoredImage {
        id:                     zoomInButton
        height:                 parent.height * 1.4
        width:                  parent.height * 1.4
        mipmap:                 true
        anchors.bottom:         parent.bottom
        anchors.bottomMargin:   _pipOverlay.height + zoomOutButton.height*0.625
        anchors.left:           parent.left
        source:                 "/qmlimages/ZoomPlus.svg"
        fillMode:               Image.PreserveAspectFit
        sourceSize.height:      height
        color:                  _colorWhite
        visible:                _showZoom

        MouseArea {
            width: 100; height: 100
            anchors.centerIn: parent

           onPressed: {
                joystickManager.cameraManagement.setSysZoomInCommand();
                zoomInButton.color = "red";
            }
            onReleased: {
                joystickManager.cameraManagement.setSysZoomStopCommand();
                zoomInButton.color = "white";
            }
        }
    }

    QGCColoredImage {
        id:                     zoomOutButton
        height:                 parent.height * 1.4
        width:                  parent.height * 1.4
        mipmap:                 true
        anchors.bottom:         parent.bottom
        anchors.bottomMargin:   _pipOverlay.height
        anchors.left:           parent.left
        source:                 "/qmlimages/ZoomMinus.svg"
        fillMode:               Image.PreserveAspectFit
        sourceSize.height:      height
        color:                  _colorWhite
        visible:                _showZoom

        MouseArea {
            width: 100; height: 100
            anchors.centerIn: parent

            onPressed: {
                joystickManager.cameraManagement.setSysZoomOutCommand();
                zoomOutButton.color = "red"
            }
            onReleased: {
                joystickManager.cameraManagement.setSysZoomStopCommand();
                zoomOutButton.color = "white"
            }
        }
    }

    NvJoystickThumbPad {
        id:                     camStick
        anchors.right:          parent.right
        anchors.bottom:         parent.bottom
        anchors.bottomMargin:   ScreenTools.isMobile ? parent.height * 0.25 : parent.height * 0.5
        anchors.leftMargin:     parent.height * 0.25
        width:                  parent.height
        height:                 parent.height
    }
}
