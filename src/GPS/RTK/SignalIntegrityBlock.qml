import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.GPS

SettingsGroupLayout {
    id: root

    required property var gps

    // Outer visibility gates on gps + at least one resilience state present.
    visible:      root.gps && root._hasResilienceData(root.gps)
    showDividers: true

    function _hasResilienceData(gps) {
        if (!gps) return false
        var jam  = gps.jammingState.value
        var spf  = gps.spoofingState.value
        var auth = gps.authenticationState.value
        return (jam  > VehicleGPSFactGroup.JammingUnknown  && jam  !== VehicleGPSFactGroup.JammingInvalid)
            || (spf  > VehicleGPSFactGroup.SpoofingUnknown && spf  !== VehicleGPSFactGroup.SpoofingInvalid)
            || (auth > VehicleGPSFactGroup.AuthUnknown     && auth !== VehicleGPSFactGroup.AuthInvalid)
    }

    function _isVisible(val, unknownVal, invalidVal) {
        return val > unknownVal && val !== invalidVal
    }

    // Warning-color helper: mitigated or worse → orange/red; ok → green
    function _integrityColor(val, okVal, mitigatedVal) {
        if (val <= okVal)                  return qgcPal.colorGreen
        if (val === mitigatedVal)          return qgcPal.colorOrange
        return qgcPal.colorRed
    }

    QGCPalette { id: qgcPal }

    property string _na: qsTr("N/A")

    RowLayout {
        spacing: ScreenTools.defaultFontPixelWidth
        visible: root.gps && root._isVisible(root.gps.jammingState.value,
                                             VehicleGPSFactGroup.JammingUnknown, VehicleGPSFactGroup.JammingInvalid)

        QGCLabel {
            text:  qsTr("Jamming")
            color: qgcPal.text
        }

        QGCLabel {
            text:  root.gps ? (root.gps.jammingState.enumStringValue || root._na) : root._na
            color: root.gps ? root._integrityColor(
                       root.gps.jammingState.value,
                       VehicleGPSFactGroup.JammingOk,
                       VehicleGPSFactGroup.JammingMitigated) : qgcPal.text
            font.bold: root.gps && root.gps.jammingState.value > VehicleGPSFactGroup.JammingOk
        }
    }

    RowLayout {
        spacing: ScreenTools.defaultFontPixelWidth
        visible: root.gps && root._isVisible(root.gps.spoofingState.value,
                                             VehicleGPSFactGroup.SpoofingUnknown, VehicleGPSFactGroup.SpoofingInvalid)

        QGCLabel {
            text:  qsTr("Spoofing")
            color: qgcPal.text
        }

        QGCLabel {
            text:  root.gps ? (root.gps.spoofingState.enumStringValue || root._na) : root._na
            color: root.gps ? root._integrityColor(
                       root.gps.spoofingState.value,
                       VehicleGPSFactGroup.SpoofingOk,
                       VehicleGPSFactGroup.SpoofingMitigated) : qgcPal.text
            font.bold: root.gps && root.gps.spoofingState.value > VehicleGPSFactGroup.SpoofingOk
        }
    }

    RowLayout {
        spacing: ScreenTools.defaultFontPixelWidth
        visible: root.gps && root._isVisible(root.gps.authenticationState.value,
                                             VehicleGPSFactGroup.AuthUnknown, VehicleGPSFactGroup.AuthInvalid)

        QGCLabel {
            text:  qsTr("Authentication")
            color: qgcPal.text
        }

        QGCLabel {
            text:  root.gps ? (root.gps.authenticationState.enumStringValue || root._na) : root._na
            color: qgcPal.text
        }
    }
}
