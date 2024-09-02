/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick                  2.12
import QtQuick.Controls         2.4
import QtQuick.Dialogs          1.3
import QtQuick.Layouts          1.12

import QtLocation               5.3
import QtPositioning            5.3
import QtQuick.Window           2.2
import QtQml.Models             2.1

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FlightDisplay 1.0
import QGroundControl.FlightMap     1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Vehicle       1.0

import QGroundControl.NextVision 1.0
import QtGraphicalEffects 1.0

Item {
    id: _root

    property var parentToolInsets               
    property var totalToolInsets:   _toolInsets
    property var mapControl
    property int    zoomValue : 0
    property var    _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property real   _toolsMargin:           ScreenTools.defaultFontPixelWidth * 0.75
    property real   _rightPanelWidth:       ScreenTools.defaultFontPixelWidth * 30

    QGCToolInsets {
        id:                         _toolInsets
        leftEdgeCenterInset:        0
        leftEdgeTopInset:           0
        leftEdgeBottomInset:        0
        rightEdgeCenterInset:       0
        rightEdgeTopInset:          0
        rightEdgeBottomInset:       0
        topEdgeCenterInset:         0
        topEdgeLeftInset:           0
        topEdgeRightInset:          0
        bottomEdgeCenterInset:      0
        bottomEdgeLeftInset:        0
        bottomEdgeRightInset:       0
    }

    Rectangle {
        id:                     _hideNvPhotoVideoControl
        anchors.right :         _nvPhotoVideoControl.right
        anchors.top:            setTopAnchors()       
        height:                 ScreenTools.isMobile ? ScreenTools.defaultFontPixelHeight*3.0 : ScreenTools.defaultFontPixelHeight*2.0
        width:                  height * 1.2
        color:                  Qt.rgba(0,0,0,0)
        anchors.margins:        0
        anchors.topMargin:      ScreenTools.isMobile ? 0 : -parent.height/5
        visible:                QGroundControl.settingsManager.flyViewSettings.showSimpleCameraControl.rawValue

        property bool show:    true

        function setTopAnchors(){
            if(ScreenTools.isMobile){
                if(instrumentPanel.visible){
                    return instrumentPanel.bottom
                }
                else{
                    return parent.top
                }
            }
            return parent.verticalCenter
        }

        function setShow(){
            show = !show
            if(show)
                _image.source = "/InstrumentValueIcons/cheveron-outline-right.svg"
            else
                _image.source = "/InstrumentValueIcons/video-camera.svg"
        }

        Image {
            id:                 _image
            width:              parent.width
            height:             parent.height
            sourceSize.height:  height
            source:             "/InstrumentValueIcons/cheveron-outline-right.svg"
            fillMode:           Image.PreserveAspectFit
            anchors.verticalCenter:     parent.verticalCenter
            anchors.horizontalCenter:   parent.horizontalCenter
        }
        MouseArea {
            anchors.fill:   parent
            onClicked:      _hideNvPhotoVideoControl.setShow()
        }
        ColorOverlay {
            anchors.fill:       _image
            source:             _image
            color:              "white"
        }
    }

    Rectangle {
        id:                     _fullScreenVideo
        anchors.right:          _hideNvPhotoVideoControl.visible ? _hideNvPhotoVideoControl.left : parent.right
        anchors.top:            setTopAnchors()
        height:                 ScreenTools.isMobile ? ScreenTools.defaultFontPixelHeight*3.0 : ScreenTools.defaultFontPixelHeight*2.0
        width:                  height * 1.2
        color:                  Qt.rgba(0,0,0,0)
        anchors.margins:        0
        anchors.topMargin:      ScreenTools.isMobile ? 0 : -parent.height/5
        visible:                QGroundControl.settingsManager.nextVisionSettings.showFullScreenButton.rawValue

        function setTopAnchors(){
            if(ScreenTools.isMobile){
                if(instrumentPanel.visible){
                    return instrumentPanel.bottom
                }
                else{
                    return parent.top
                }
            }
            return parent.verticalCenter
        }

        Image {
            id:                 _imageFs
            width:              parent.width
            height:             parent.height
            sourceSize.height:  height
            source:             "/InstrumentValueIcons/screen-full.svg"
            fillMode:           Image.PreserveAspectFit
            anchors.verticalCenter:     parent.verticalCenter
            anchors.horizontalCenter:   parent.horizontalCenter
        }
        MouseArea {
            anchors.fill:   parent
            onClicked:      {
                if(videoControl !== null){
                    if(videoControl.pipState.state === videoControl.pipState.fullState){
                        QGroundControl.videoManager.fullScreen = true
                    }
                    else{
                        _pipOverlay._swapPip()
                        QGroundControl.videoManager.fullScreen = true
                    }
                }
            }
        }
        ColorOverlay {
            anchors.fill:       _imageFs
            source:             _imageFs
            color:              "white"
        }
    }

    NvPhotoVideoControl {
        id:                     _nvPhotoVideoControl
        anchors.margins:        _toolsMargin
        anchors.top:            _hideNvPhotoVideoControl.bottom
        anchors.topMargin:      0
        anchors.right:          parent.right
        width:                  _rightPanelWidth
        visible:                QGroundControl.settingsManager.flyViewSettings.showSimpleCameraControl.rawValue && _hideNvPhotoVideoControl.show
    }

    Loader {
        id:                         virtualCamJoystickMultiTouch
        z:                          QGroundControl.zOrderTopMost + 1
        width:                      parent.width
        height:                     Math.min(parent.height * 0.25, ScreenTools.defaultFontPixelWidth * 16)
        visible:                    _virtualCamJoystickEnabled && !QGroundControl.videoManager.fullScreen && !(_activeVehicle ? _activeVehicle.usingHighLatencyLink : false)
        anchors.bottom:             parent.bottom
        source:                     "qrc:/nvqml/QGroundControl/NextVision/NvVirtualCamJoystick.qml"
        active:                     _virtualCamJoystickEnabled && !(_activeVehicle ? _activeVehicle.usingHighLatencyLink : false)

        property bool _virtualCamJoystickEnabled: QGroundControl.settingsManager.nextVisionSettings.virtualCamJoystick.rawValue
    }
}
