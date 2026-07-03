import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.GPS

/// Colored circle indicating GPS fix or general status.
/// Pick ONE of:
///   - `lockValue`   — GPS fix-type lock (uses GPSFormatter.fixTypeColor)
///   - `severity`    — "Info" | "Warning" | "Error" (e.g. event log entries)
///   - `statusColor` — set directly (other status semantics)
Item {
    id: root

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
        if (root.lockValue < 0) return _pal.colorGrey
        var c = GPSFormatter.fixTypeColor(root.lockValue)
        if (c === "green")  return _pal.colorGreen
        if (c === "orange") return _pal.colorOrange
        if (c === "red")    return _pal.colorRed
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
