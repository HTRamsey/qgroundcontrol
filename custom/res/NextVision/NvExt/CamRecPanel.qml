import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles  1.4
import QtQuick.Dialogs  1.2
import QtQuick.Layouts  1.2

import QGroundControl               1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Controllers   1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl.SettingsManager 1.0

Rectangle {
    width:              parent.width
    height:             parent.height
    color:              Qt.rgba(0.0,0.0,0.0,0.25)
    visible:            true

    property var    _activeVehicle:             QGroundControl.multiVehicleManager.activeVehicle

    // The following properties relate to a simple camera
    property var    _nextVisionSettings:                           QGroundControl.settingsManager.nextVisionSettings
    property bool   _simpleCameraAvailable:                     _activeVehicle && _nextVisionSettings.showCameraControl.rawValue
    property bool   _onlySimpleCameraAvailable:                 !_anyVideoStreamAvailable && _simpleCameraAvailable
    property bool   _simpleCameraIsShootingInCurrentMode:       _onlySimpleCameraAvailable && !_simplePhotoCaptureIsIdle

    // The following properties relate to a simple video stream
    property bool   _videoStreamAvailable:                      _videoStreamManager.hasVideo
    property var    _videoStreamSettings:                       QGroundControl.settingsManager.videoSettings
    property var    _videoStreamManager:                        QGroundControl.videoManager
    property bool   _videoStreamAllowsPhotoWhileRecording:      true
    property bool   _videoStreamIsStreaming:                    _videoStreamManager.streaming
    property bool   _simplePhotoCaptureIsIdle:             true
    property bool   _videoStreamRecording:                      _videoStreamManager.recording
    property bool   _videoStreamCanShoot:                       _videoStreamIsStreaming
    property bool   _videoStreamIsShootingInCurrentMode:        _videoStreamInPhotoMode ? !_simplePhotoCaptureIsIdle : _videoStreamRecording
    property bool   _videoStreamInPhotoMode:                    false

    property bool   _anyVideoStreamAvailable:                   _videoStreamManager.hasVideo
    property string _cameraName:                                ""
    property bool   _showModeIndicator:                         _videoStreamManager.hasVideo
    property bool   _modeIndicatorPhotoMode:                    _videoStreamInPhotoMode || _onlySimpleCameraAvailable
    property bool   _allowsPhotoWhileRecording:                 _videoStreamAllowsPhotoWhileRecording
    property bool   _switchToPhotoModeAllowed:                  !_modeIndicatorPhotoMode
    property bool   _switchToVideoModeAllowed:                  _modeIndicatorPhotoMode
    property bool   _videoIsRecording:                          _videoStreamRecording
    property bool   _canShootInCurrentMode:                     _videoStreamCanShoot || _simpleCameraAvailable
    property bool   _isShootingInCurrentMode:                   _videoStreamIsShootingInCurrentMode || _simpleCameraIsShootingInCurrentMode

    function toggleShooting() {
        console.log("toggleShooting", _anyVideoStreamAvailable)
        if (_anyVideoStreamAvailable) {
            if (_videoStreamManager.recording) {
                _videoStreamManager.stopRecording()
            } else {
                _videoStreamManager.startRecording()
            }
        }
    }



    QGCLabel {
        id:                 mainlabel
        text:               qsTr("REC")
        anchors.margins:    ScreenTools.isMobile ? _margins * 1.6 : _margins
        font.family:        ScreenTools.demiboldFontFamily
        font.pointSize:     ScreenTools.largeFontPointSize
        anchors.horizontalCenter:  parent.horizontalCenter
        anchors.top:        parent.top
        color:              "white"
        height:             ScreenTools.defaultFontPixelHeight
    }

    QGCLabel {
        id:                 lrlabel
        text:               qsTr("Local Recording")
        anchors.margins:    _margins * 4
        font.family:        ScreenTools.demiboldFontFamily
        font.pointSize:     ScreenTools.isMobile? point_size : ScreenTools.mediumFontPointSize
        anchors.horizontalCenter:  parent.horizontalCenter
        anchors.top:        mainlabel.bottom
        height:             ScreenTools.smallFontPointSize
        color:              "white"
    }

    Rectangle {
        id:                 localrec
        Layout.alignment:   Qt.AlignHCenter
        color:              Qt.rgba(0,0,0,0)
        width:              ScreenTools.defaultFontPixelWidth * 6
        height:             width
        radius:             width * 0.5
        border.color:       qgcPal.buttonText
        border.width:       3
        anchors.margins:    _margins * 2
        anchors.horizontalCenter:  parent.horizontalCenter
        anchors.top:        lrlabel.bottom

        Rectangle {
            anchors.centerIn:   parent
            width:              parent.width * (_isShootingInCurrentMode ? 0.5 : 0.75)
            height:             width
            radius:             _isShootingInCurrentMode ? 0 : width * 0.5
            color:              _canShootInCurrentMode ? qgcPal.colorRed : qgcPal.colorGrey
        }

        MouseArea {
            anchors.fill:   parent
            enabled:        _canShootInCurrentMode
            onClicked:      toggleShooting()
        }
    }

    QGCLabel {
        id:                 rrlabel
        text:               qsTr("Remote Recording")
        anchors.margins:    _margins
        font.family:        ScreenTools.demiboldFontFamily
        font.pointSize:     ScreenTools.isMobile? point_size : ScreenTools.mediumFontPointSize
        anchors.horizontalCenter:  parent.horizontalCenter
        anchors.top:        localrec.bottom
        height:             ScreenTools.smallFontPointSize
        color:              "white"
    }

    RowLayout {
        id:                         firstRow
        anchors.horizontalCenter:   parent.horizontalCenter
        anchors.top:                rrlabel.bottom
        anchors.margins:            _margins * 2
        spacing:                    _butMargins
        visible:                    true

        QGCButton {
            showBorder:     true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
            pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
            text:           qsTr("REC On")
            leftPadding:    0
            rightPadding:   0
            onReleased: {
                joystickManager.cameraManagement.setSysRecOnCommand();
            }
        }
        QGCButton {
            showBorder:     true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
            pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
            text:           qsTr("REC Off")
            leftPadding:    0
            rightPadding:   0
            onReleased: {
                joystickManager.cameraManagement.setSysRecOffCommand();
            }
        }
        QGCButton {
            showBorder:     true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
            pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
            text:           qsTr("Snap")
            leftPadding:    0
            rightPadding:   0
            onReleased: {
                joystickManager.cameraManagement.setSysSnapshotCommand();
            }
        }
    }

    QGCLabel {
        id:                 aslabel
        text:               qsTr("Auto Snapshots")
        anchors.margins:    _margins
        font.family:        ScreenTools.demiboldFontFamily
        font.pointSize:     ScreenTools.isMobile? point_size : ScreenTools.mediumFontPointSize
        anchors.horizontalCenter:  parent.horizontalCenter
        anchors.top:        firstRow.bottom
        height:             ScreenTools.smallFontPointSize
        color:              "white"
    }

    RowLayout {
        id:                         secondRow
        anchors.top:                aslabel.bottom
        anchors.margins:            _margins * 2
        spacing:                    _butMargins * 3
        visible:                    true

        QGCLabel {
            text:                   qsTr(" Interval[ms]")
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            color:                  "white"
        }
        QGCTextField {
            id:                     _snapInerval
            Layout.preferredWidth:  ScreenTools.isMobile ? 90 : 45
            maximumLength:          5
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            text:                   qsTr("1000")
        }
        QGCLabel {
            id:                     _snapStatus
            text:                   qsTr("Idle")
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            color:                  "white"

            Connections {
                target: _activeVehicle
                onSnapShotStatusChanged: {
                    _snapStatus.text = snapShotStatus ? qsTr("Busy") : qsTr("Idle")
                }
            }
        }
    }

    RowLayout {
        id:                         thirdRow
        anchors.top:                secondRow.bottom
        spacing:                    _butMargins
        anchors.margins:            _margins
        visible:                    true

        QGCLabel {
            text:                   qsTr(" Count")
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            color:                  "white"
        }
        QGCTextField {
            id:                     _snapCount
            Layout.preferredWidth:  ScreenTools.isMobile ? 90 : 40
            maximumLength:          4
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            text:                   qsTr("10")
        }
        QGCLabel {
            text:                   qsTr("   ")
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
        }
        QGCCheckBox {
            id:                     _snapInf
            text:                   qsTr("Inf")
            textColor:              "white"
            textFontPointSize:      ScreenTools.isMobile ? point_size : 9
        }
    }

    RowLayout {
        id:                         lastRow
        anchors.horizontalCenter:   parent.horizontalCenter
        anchors.top:                thirdRow.bottom
        spacing:                    _butMargins
        anchors.margins:            _margins
        visible:                    true

        QGCButton {
            showBorder:     true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
            pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
            text:           qsTr("Start")
            leftPadding:    0
            rightPadding:   0
            onReleased: {
                joystickManager.cameraManagement.setSysAutoSnapshotCommand(_snapInerval.text,_snapCount.text,_snapInf.checked);
            }
        }
        QGCButton {
            showBorder:     true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
            pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
            text:           qsTr("Stop")
            leftPadding:    0
            rightPadding:   0
            onReleased: {
                joystickManager.cameraManagement.setSysAutoSnapshotCommand(0,0,_snapInf.checked);
            }
        }
    }
}


