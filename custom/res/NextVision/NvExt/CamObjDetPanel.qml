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

    property var _odNetTypes: ["Human & Vehicle", "Fire & Smoke", "Human Overboard", "Marine Vessel"]

    QGCLabel {
        id:                 mainlabel
        text:               qsTr("OBJ DET")
        anchors.margins:    ScreenTools.isMobile ? _margins * 1.6 : _margins
        font.family:        ScreenTools.demiboldFontFamily
        font.pointSize:     ScreenTools.largeFontPointSize
        anchors.horizontalCenter:  parent.horizontalCenter
        anchors.top:        parent.top
        height:             ScreenTools.defaultFontPixelHeight
        color:              "White"
    }

    RowLayout {
        id:                         firstRow
        anchors.horizontalCenter:   parent.horizontalCenter
        anchors.top:                mainlabel.bottom
        anchors.margins:            _margins * 4
        spacing:                    _butMargins
        visible:                    true

        QGCButton {
            showBorder:     true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
            pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
            text:           qsTr("OD Enable")
            leftPadding:    0
            rightPadding:   0            
            onReleased: {
                joystickManager.cameraManagement.setSysObjDetOnCommand();
            }
        }
        QGCButton {
            showBorder:     true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
            pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
            text:           qsTr("OD Disable")
            leftPadding:    0
            rightPadding:   0
            onReleased: {
                joystickManager.cameraManagement.setSysObjDetOffCommand();
            }
        }
    }

    QGCLabel {
        id:                 detectorTypeLabel
        text:               qsTr("Detector Type")
        anchors.margins:    _margins
        font.family:        ScreenTools.demiboldFontFamily
        font.pointSize:     ScreenTools.isMobile ? point_size : ScreenTools.mediumFontPointSize
        anchors.horizontalCenter:  parent.horizontalCenter
        anchors.top:        firstRow.bottom
        color:              "white"
        height:             ScreenTools.smallFontPointSize
    }

    RowLayout {
        id:                         secondRow
        anchors.horizontalCenter:   parent.horizontalCenter
        anchors.top:                detectorTypeLabel.bottom
        anchors.margins:            _margins * 2
        spacing:                    _butMargins
        visible:                    true

        QGCComboBox {
            id:             netTypeCombo
            sizeToContents: true
            centeredLabel:  true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.mediumFontPointSize
            leftPadding:    0
            rightPadding:   0
            model:          _odNetTypes            
        }

        QGCButton {
            showBorder:     true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
            pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
            text:           qsTr("Set")
            leftPadding:    0
            rightPadding:   0
            onReleased: {
                if(netTypeCombo.currentIndex >=0 )
                    joystickManager.cameraManagement.setSysObjDetSetNetTypeCommand(netTypeCombo.currentIndex);
            }
        }
    }

    RowLayout {
        id:                         thirdRow
        anchors.horizontalCenter:   parent.horizontalCenter
        anchors.top:                secondRow.bottom
        anchors.margins:            _margins
        spacing:                    _butMargins
        visible:                    true

        QGCLabel {
            text:                   qsTr("Conf Thres[%]")
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            color:                  "White"
        }

        QGCTextField {
            id:                     _confThres
            Layout.preferredWidth:  ScreenTools.isMobile ? 90 : 45
            maximumLength:          6
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            text:                   qsTr("")
        }

        QGCButton {
            showBorder:                 true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
            pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
            text:                       qsTr("Set")
            leftPadding:                0
            rightPadding:               0
            onReleased: {
                joystickManager.cameraManagement.setSysObjDetSetConfThresCommand(_confThres.text);
            }
        }
    }

    RowLayout {
        id:                         lastRow
        anchors.top:                thirdRow.bottom
        anchors.horizontalCenter:   parent.horizontalCenter
        anchors.margins:            _margins
        spacing:                    _butMargins
        visible:                    true

        QGCLabel {
            text:                   qsTr("Fire Thres   ")
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            color:                  "White"
        }

        QGCTextField {
            id:                     _fireThres
            Layout.preferredWidth:  ScreenTools.isMobile ? 90 : 45
            maximumLength:          6
            font.pointSize:         ScreenTools.isMobile ? point_size : 9
            text:                   qsTr("")
        }

        QGCButton {
            showBorder:                 true
            font.pointSize: ScreenTools.isMobile? point_size : ScreenTools.smallFontPointSize
            pointSize:      ScreenTools.isMobile? point_size : ScreenTools.defaultFontPointSize
            text:                       qsTr("Set")
            leftPadding:                0
            rightPadding:               0
            onReleased: {
                joystickManager.cameraManagement.setSysObjDetSetFireThresCommand(_fireThres.text);
            }
        }
    }
}
