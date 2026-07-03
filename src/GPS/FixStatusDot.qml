import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls

/// Colored circle indicating GPS fix or general status.
/// Pick ONE of:
///   - `quality`     — derived GPS quality tier (VehicleGPSFactGroup.GPSQuality)
///   - `severity`    — "Info" | "Warning" | "Error" (e.g. event log entries)
///   - `statusColor` — set directly (other status semantics)
/// `lockValue` may additionally be set to show a fix-type tooltip.
Item {
    id: root

    property int    quality:     -1
    property int    lockValue:   -1
    property string severity:    ""
    property color  statusColor: _resolvedColor
    property real   dotSize:     ScreenTools.defaultFontPixelHeight * 0.6

    Layout.preferredWidth:  root.dotSize
    Layout.preferredHeight: root.dotSize
    Layout.alignment:       Qt.AlignVCenter

    implicitWidth:  root.dotSize
    implicitHeight: root.dotSize

    QGCPalette { id: _pal }

    property color _resolvedColor: {
        if (root.severity !== "") {
            if (root.severity === "Error")   return _pal.colorRed
            if (root.severity === "Warning") return _pal.colorOrange
            return _pal.colorGreen
        }
        if (root.quality >= VehicleGPSFactGroup.QualityGood) return _pal.colorGreen
        if (root.quality >= VehicleGPSFactGroup.QualityFair) return _pal.colorOrange
        if (root.quality >= VehicleGPSFactGroup.QualityPoor) return _pal.colorRed
        return _pal.colorGrey
    }

    Rectangle {
        anchors.fill: parent
        radius:       width / 2
        color:        root.statusColor

        ToolTip.visible: _hov.containsMouse && root.lockValue >= 0
        ToolTip.delay:   600
        ToolTip.text: {
            if (root.lockValue >= VehicleGPSFactGroup.FixRTKFixed) return qsTr("RTK Fixed — centimeter accuracy")
            if (root.lockValue >= VehicleGPSFactGroup.FixRTKFloat) return qsTr("RTK Float — sub-meter accuracy")
            if (root.lockValue >= VehicleGPSFactGroup.Fix3D)       return qsTr("3D Fix — meter-level accuracy")
            if (root.lockValue >= VehicleGPSFactGroup.Fix2D)       return qsTr("2D Fix — altitude unreliable")
            return qsTr("No Fix")
        }

        HoverHandler { id: _hov }
    }
}
