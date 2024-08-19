/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick 2.12

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.Controllers   1.0
import QGroundControl.ScreenTools   1.0
import QtGraphicalEffects 1.0

Item {
    id:         _root
    visible:    QGroundControl.videoManager.hasVideo

    property Item pipState: videoPipState
    QGCPipState {
        id:         videoPipState
        pipOverlay: _pipOverlay
        isDark:     true

        onWindowAboutToOpen: {
            QGroundControl.videoManager.stopVideo()
            videoStartDelay.start()
        }

        onWindowAboutToClose: {
            QGroundControl.videoManager.stopVideo()
            videoStartDelay.start()
        }

        onStateChanged: {
            if (pipState.state !== pipState.fullState) {
                QGroundControl.videoManager.fullScreen = false
            }
        }
    }

    Timer {
        id:           videoStartDelay
        interval:     2000;
        running:      false
        repeat:       false
        onTriggered:  QGroundControl.videoManager.startVideo()
    }

    //-- Video Streaming
    FlightDisplayViewVideo {
        id:             videoStreaming
        anchors.fill:   parent
        useSmallFont:   _root.pipState.state !== _root.pipState.fullState
        visible:        QGroundControl.videoManager.isGStreamer
    }
    //-- UVC Video (USB Camera or Video Device)
    Loader {
        id:             cameraLoader
        anchors.fill:   parent
        visible:        !QGroundControl.videoManager.isGStreamer
        source:         visible ? (QGroundControl.videoManager.uvcEnabled ? "qrc:/qml/FlightDisplayViewUVC.qml" : "qrc:/qml/FlightDisplayViewDummy.qml") : ""
    }

    MouseArea {
        anchors.fill:       parent
        enabled:            pipState.state === pipState.fullState
        onDoubleClicked: {            
            QGroundControl.videoManager.fullScreen = !QGroundControl.videoManager.fullScreen
        }
        onClicked: {
            /* Calculating the position to track on */
            var videoWidth
            var videoHeight
            var videoMargin
            var xPos = mouseX
            videoHeight = height
            videoWidth = (videoHeight * 16.0 ) / 9.0
            videoMargin = (width - videoWidth) / 2.0
            if(mouseX < (videoMargin + 16))
                xPos = videoMargin + 16
            else if(mouseX > (videoMargin + videoWidth - 16) )
                xPos = (videoMargin + videoWidth - 16)
            xPos -= videoMargin
            var xScaled = (1280.0 * xPos) / videoWidth
            var yScaled = (720.0 * mouseY) / videoHeight
            /* Sending the Track On Position command to the TRIP */
            joystickManager.cameraManagement.trackOnPosition(xScaled,yScaled);
        }
    }

    Rectangle {
        id:                     _exitFullScreenVideo
        anchors.right :         parent.right
        anchors.top:            parent.top
        height:                 ScreenTools.isMobile ? ScreenTools.defaultFontPixelHeight*3.0 : ScreenTools.defaultFontPixelHeight*2.0
        width:                  height * 1.2
        color:                  Qt.rgba(0,0,0,0)
        anchors.margins:        ScreenTools.defaultFontPixelWidth * 0.75
        visible:                QGroundControl.videoManager.fullScreen

        Image {
            id:                 _imageExit
            width:              parent.width
            height:             parent.height
            sourceSize.height:  height
            source:             "/InstrumentValueIcons/close-outline.svg"
            fillMode:           Image.PreserveAspectFit
            anchors.verticalCenter:     parent.verticalCenter
            anchors.horizontalCenter:   parent.horizontalCenter
        }
        MouseArea {
            anchors.fill:   parent
            onClicked:      {
                QGroundControl.videoManager.fullScreen = false
            }
        }
        ColorOverlay {
            anchors.fill:       _imageExit
            source:             _imageExit
            color:              "white"
        }
    }

    ProximityRadarVideoView{
        anchors.fill:   parent
        vehicle:        QGroundControl.multiVehicleManager.activeVehicle
    }
}
