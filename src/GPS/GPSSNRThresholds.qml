pragma Singleton
import QtQuick

/// SNR quality thresholds used by SignalStrengthBar and its legend.
/// strong: signals at or above this value are shown green ("strong lock")
/// weak:   signals at or above this value (but below strong) are shown orange ("usable")
/// signals below weak are shown red ("poor / will lose lock")
QtObject {
    readonly property real strong: 35
    readonly property real weak:   20
}
