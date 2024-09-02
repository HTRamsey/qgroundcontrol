/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick                  2.4
import QtPositioning            5.2
import QtQuick.Layouts          1.2
import QtQuick.Controls         1.4
import QtQuick.Dialogs          1.2
import QtGraphicalEffects       1.0

import QGroundControl                   1.0
import QGroundControl.ScreenTools       1.0
import QGroundControl.Controls          1.0
import QGroundControl.Palette           1.0
import QGroundControl.Vehicle           1.0
import QGroundControl.Controllers       1.0
import QGroundControl.FactSystem        1.0
import QGroundControl.FactControls      1.0

Rectangle {
    id:         _nvPhotoVideoControl
    height:     pagesHeights[_panel_index]
    color:      "#80000000"
    radius:     _margins    

    property real   _margins:                                   ScreenTools.defaultFontPixelHeight / 2
    property real   _butMargins:                                ScreenTools.defaultFontPixelHeight / 4    

    /* nextvision panel resources */
    property int    _panel_index:                               0
    property int    _panel_index_min:                           0
    property int    _panel_index_max:                           8
    property real   point_size:     point_sizes[QGroundControl.settingsManager.nextVisionSettings.camControlFontSize.rawValue]
    property var    point_sizes:    [ScreenTools.smallFontPointSize, ScreenTools.mediumFontPointSize, ScreenTools.largeFontPointSize]
    property int    mainLayout_height:                          151 //replaced mainlayout.height

    function updatePanelIndex(dir) {
        if ( _panel_index + dir >= _panel_index_max )
            _panel_index = 0;
        else if ( _panel_index + dir < _panel_index_min )
            _panel_index = _panel_index_max - 1;
        else
            _panel_index += dir
    }

    QGCPalette { id: qgcPal; colorGroupEnabled: enabled }    

    property var camQuickPages: ["qrc:/nvqml/CamQuickPanel.qml","qrc:/nvqml/CamQuick1Panel.qml","qrc:/nvqml/CamQuick2Panel.qml"]
    property var camQuickpagesHeights:  [mainLayout_height + (_margins * 10), mainLayout_height + (_margins * 10), mainLayout_height + (_margins * 9)]
    property var pages:         [camQuickPages[QGroundControl.settingsManager.nextVisionSettings.quickViewMode.rawValue],
        "qrc:/nvqml/CamModePanel.qml", "qrc:/nvqml/CamIRPanel.qml", "qrc:/nvqml/CamRecPanel.qml",
        "qrc:/nvqml/CamPosPanel.qml", "qrc:/nvqml/CamFModePanel.qml", "qrc:/nvqml/CamObjDetPanel.qml",
        "qrc:/nvqml/CamMiscPanel.qml"]
    property var pagesHeights:  [camQuickpagesHeights[QGroundControl.settingsManager.nextVisionSettings.quickViewMode.rawValue]*pagesHeightsMul,
        (mainLayout_height + ( 10 * _margins))*pagesHeightsMul,(mainLayout_height + ( 5 * _margins ))*pagesHeightsMul,(mainLayout_height + ( 23 * _margins ))*pagesHeightsMul,
        (mainLayout_height + ( 14 * _margins ))*pagesHeightsMul,(mainLayout_height + ( 5 * _margins ))*pagesHeightsMul, (mainLayout_height + ( 14 * _margins ))*pagesHeightsMul,
        (mainLayout_height/2 + ( 4 * _margins ))*pagesHeightsMul]
    property var pagesHeightsMul: pagesHeightsMuls[ScreenTools.isMobile? ( QGroundControl.settingsManager.nextVisionSettings.camControlFontSize.rawValue + 1 ) : 0]
    property var pagesHeightsMuls: [1.0,1.1,1.2,1.3]

    Loader {
        source:             pages[_panel_index]
        height:             parent.height
        width:              parent.width
    }

    QGCColoredImage {
        anchors.margins:    _margins
        anchors.top:        parent.top
        anchors.right:      parent.right
        source:             "/res/buttonRight.svg"
        height:             ScreenTools.isMobile ? ScreenTools.defaultFontPixelHeight * 2.0 : ScreenTools.defaultFontPixelHeight
        width:              height * 1.2
        sourceSize.height:  height * 1.2
        color:              "white"
        fillMode:           Image.PreserveAspectFit
        visible:            true

        QGCMouseArea {
            fillItem:   parent
            onClicked:
                  updatePanelIndex(1)
        }
    }

    QGCColoredImage {
        anchors.margins:    _margins
        anchors.top:        parent.top
        anchors.left:       parent.left
        source:             "/res/buttonLeft.svg"
        height:             ScreenTools.isMobile ? ScreenTools.defaultFontPixelHeight * 2.0 : ScreenTools.defaultFontPixelHeight
        width:              height * 1.2
        sourceSize.height:  height * 1.2
        color:              "white"
        fillMode:           Image.PreserveAspectFit
        visible:            true

        QGCMouseArea {
            fillItem:   parent
            onClicked:
                updatePanelIndex(-1)
        }
    }
}
