import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.GPS
/// Status block for the RTK corrections pipeline (NTRIP + GCS-connected RTK base).
/// Shows current corrections state (RTK Fixed / Float / Waiting / No Fix), source,
/// correction age, and a warning when corrections have been streaming with no RTK
/// achieved. Drop-in for GPSIndicatorPage and anywhere else that needs the same
/// RTK-side view of vehicle GPS.
SettingsGroupLayout {
    id: root

    required property var vehicle

    /// Whether RTCM corrections are flowing from any source. Derived from the same
    /// globals the rest of this block reads, so it needs no caller-side wiring.
    readonly property bool correctionsActive: QGroundControl.gpsRtk.connected.value
        || (root._ntripMgr && root._ntripMgr.connectionStatus === NTRIPManager.Connected)

    property string valueNA: qsTr("-.--")

    readonly property int  _vehicleLock:   root.vehicle ? root.vehicle.gps.lock.rawValue : 0
    readonly property bool _vehicleHasRtk: root._vehicleLock >= VehicleGPSFactGroup.FixRTKFloat
    readonly property var  _ntripMgr:      QGroundControl.ntripManager

    // Seconds corrections have been flowing without an RTK fix; gates the
    // "No RTK Fix" warning after kNoFixWarningSec.
    property int  _correctionsSentSec: 0
    readonly property int kNoFixWarningSec: 30

    heading:      qsTr("RTK Corrections")
    showDividers: true
    visible:      root.vehicle && root.correctionsActive

    QGCPalette { id: _pal }

    Timer {
        interval:         1000
        running:          root.correctionsActive && !root._vehicleHasRtk
        repeat:           true
        onTriggered:      root._correctionsSentSec++
        onRunningChanged: root._correctionsSentSec = 0
    }

    RowLayout {
        spacing: ScreenTools.defaultFontPixelWidth

        FixStatusDot {
            statusColor: {
                if (root._vehicleLock >= VehicleGPSFactGroup.FixRTKFixed)    return _pal.colorGreen
                if (root._vehicleLock >= VehicleGPSFactGroup.FixRTKFloat)    return _pal.colorOrange
                if (root.correctionsActive && root._correctionsSentSec > root.kNoFixWarningSec) return _pal.colorRed
                if (root.correctionsActive)                                  return _pal.colorOrange
                return _pal.colorGrey
            }
        }

        QGCLabel {
            text: {
                if (root._vehicleLock >= VehicleGPSFactGroup.FixRTKFixed)    return qsTr("RTK Fixed")
                if (root._vehicleLock >= VehicleGPSFactGroup.FixRTKFloat)    return qsTr("RTK Float")
                if (root.correctionsActive && root._correctionsSentSec > root.kNoFixWarningSec) return qsTr("No RTK Fix")
                if (root.correctionsActive)                                  return qsTr("Waiting for RTK...")
                return qsTr("No Corrections")
            }
        }
    }

    LabelledLabel {
        label:     qsTr("Source")
        labelText: {
            var sources = []
            if (root._ntripMgr && root._ntripMgr.connectionStatus === NTRIPManager.Connected)
                sources.push(qsTr("NTRIP"))
            if (QGroundControl.gpsRtk.connected.value)
                sources.push(qsTr("RTK Base"))
            return sources.join(" + ")
        }
    }

    LabelledLabel {
        label:     qsTr("Data Rate")
        labelText: {
            var stats = root._ntripMgr ? root._ntripMgr.connectionStats : null
            if (!stats || stats.dataRateBytesPerSec < 0) return root.valueNA
            return GPSFormatter.formatDataRate(stats.dataRateBytesPerSec)
        }
        visible: root._ntripMgr && root._ntripMgr.connectionStatus === NTRIPManager.Connected
    }

    LabelledLabel {
        label:     qsTr("Correction Age")
        labelText: {
            var stats = root._ntripMgr ? root._ntripMgr.connectionStats : null
            if (!stats || stats.correctionAgeSec < 0) return root.valueNA
            var age = stats.correctionAgeSec
            if (age < 1.0) return qsTr("< 1 s")
            return age.toFixed(0) + " s"
        }
        visible: root._ntripMgr && root._ntripMgr.connectionStatus === NTRIPManager.Connected
    }

    LabelledLabel {
        label:     qsTr("RTK Satellites")
        labelText: root.vehicle ? root.vehicle.gps.rtkNumSats.valueString : root.valueNA
        visible:   root.vehicle && root.vehicle.gps.rtkNumSats.rawValue > 0
    }

    LabelledLabel {
        label:     qsTr("Correction Rate")
        labelText: root.vehicle ? root.vehicle.gps.rtkRate.valueString + " Hz" : root.valueNA
        visible:   root.vehicle && root.vehicle.gps.rtkRate.rawValue > 0
    }

    LabelledLabel {
        label:     qsTr("RTK Health")
        labelText: root.vehicle ? root.vehicle.gps.rtkHealth.valueString : root.valueNA
        visible:   root.vehicle && root.vehicle.gps.rtkRate.rawValue > 0
    }

    LabelledLabel {
        label:     qsTr("Baseline")
        labelText: root.vehicle ? root.vehicle.gps.rtkBaseline.valueString + " m" : root.valueNA
        visible:   root.vehicle && !isNaN(root.vehicle.gps.rtkBaseline.rawValue)
    }

    LabelledLabel {
        label:     qsTr("RTK Accuracy")
        labelText: root.vehicle ? root.vehicle.gps.rtkAccuracy.valueString + " m" : root.valueNA
        visible:   root.vehicle && !isNaN(root.vehicle.gps.rtkAccuracy.rawValue)
    }

    LabelledLabel {
        label:     qsTr("Ambiguity Hypotheses")
        labelText: root.vehicle ? root.vehicle.gps.rtkIAR.valueString : root.valueNA
        visible:   root.vehicle && root.vehicle.gps.rtkIAR.rawValue > 0
    }

    QGCLabel {
        Layout.fillWidth: true
        text:             qsTr("Corrections are being sent but vehicle has not achieved RTK fix. Check base station accuracy, satellite count, and signal quality.")
        wrapMode:         Text.WordWrap
        color:            _pal.colorOrange
        font.pointSize:   ScreenTools.smallFontPointSize
        visible:          root.correctionsActive && !root._vehicleHasRtk && root._correctionsSentSec > root.kNoFixWarningSec
    }
}
