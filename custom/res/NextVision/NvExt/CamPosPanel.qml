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

    property Fact _localPositionRoll: QGroundControl.settingsManager.nextVisionSettings.localPositionRoll
    property Fact _localPositionPitch: QGroundControl.settingsManager.nextVisionSettings.localPositionPitch
    property Fact _globalPositionElevation: QGroundControl.settingsManager.nextVisionSettings.globalPositionElevation
    property Fact _globalPositionAzimuth: QGroundControl.settingsManager.nextVisionSettings.globalPositionAzimuth

    QGCLabel {
        id:                 mainlabel
        text:               qsTr("POSITION")
        anchors.margins:    ScreenTools.isMobile ? _margins * 1.6 : _margins
        font.family:        ScreenTools.demiboldFontFamily
        font.pointSize:     ScreenTools.largeFontPointSize
        anchors.horizontalCenter:  parent.horizontalCenter
        anchors.top:        parent.top
        color:              "white"
        height:             ScreenTools.defaultFontPixelHeight
    }

    QGCLabel {
        id:                 localposlabel
        text:               qsTr("Local")
        anchors.margins:    _margins * 4
        font.family:        ScreenTools.demiboldFontFamily
        font.pointSize:     ScreenTools.isMobile? point_size : ScreenTools.mediumFontPointSize
        anchors.horizontalCenter:  parent.horizontalCenter
        anchors.top:        mainlabel.bottom
        color:              "white"
        height:             ScreenTools.smallFontPointSize
    }


    RowLayout {
        id:                         firstRow
        anchors.top:                localposlabel.bottom
        anchors.margins:            _margins * 2
        visible:                    true

        QGCLabel {
            text:                   qsTr(" Roll")
            color:                  "white"
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
        }

        FactTextField {
            id:                     _localPosRoll
            Layout.preferredWidth:  ScreenTools.isMobile ? 90 : 45
            maximumLength:          6
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            fact:                   _localPositionRoll
        }

        QGCLabel {
            text:                   qsTr("Pitch")
            color:                  "white"
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
        }

        FactTextField {
            id:                     _localPosPitch
            Layout.preferredWidth:  ScreenTools.isMobile ? 90 : 45
            maximumLength:          6
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            fact:                   _localPositionPitch
        }
    }

    QGCButton {
        id:                         localPosBtn
        showBorder:                 true
        anchors.margins:            _margins
        anchors.top:                firstRow.bottom
        font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
        pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
        text:                       qsTr("Set Local Position")
        anchors.horizontalCenter:   parent.horizontalCenter
        leftPadding:                0
        rightPadding:               0

        onReleased: {
            joystickManager.cameraManagement.setSysModeLocalPositionCommand(_localPosPitch.text, _localPosRoll.text);
        }
    }


    QGCLabel {
        id:                 globalposlabel
        text:               qsTr("Global")
        anchors.margins:    _margins
        font.family:        ScreenTools.demiboldFontFamily
        font.pointSize:     ScreenTools.isMobile ? point_size : ScreenTools.mediumFontPointSize
        anchors.horizontalCenter:  parent.horizontalCenter
        anchors.top:        localPosBtn.bottom
        color:              "white"
        height:             ScreenTools.smallFontPointSize
    }

    RowLayout {
        id:                         secondRow
        anchors.top:                globalposlabel.bottom
        anchors.margins:            _margins * 2
        visible:                    true

        QGCLabel {
            text:                   qsTr(" Elev")
            color:                  "white"
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
        }

        FactTextField {
            id:                     _globalPosElev
            Layout.preferredWidth:  ScreenTools.isMobile ? 90 : 45
            maximumLength:          6
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            fact:                   _globalPositionElevation
        }

        QGCLabel {
            text:                   qsTr("Az")
            color:                  "white"
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
        }

        FactTextField {
            id:                     _globalPosAz
            Layout.preferredWidth:  ScreenTools.isMobile ? 90 : 45
            maximumLength:          6
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            fact:                   _globalPositionAzimuth
        }
    }

    QGCButton {
        showBorder:                 true
        anchors.margins:            _margins
        anchors.top:                secondRow.bottom
        font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
        pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
        text:                       qsTr("Set Global Position")
        anchors.horizontalCenter:   parent.horizontalCenter
        leftPadding:                0
        rightPadding:               0

        onReleased: {
            joystickManager.cameraManagement.setSysModeGlobalPositionCommand(_globalPosElev.text, _globalPosAz.text);
        }
    }
}


