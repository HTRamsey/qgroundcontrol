pragma Singleton

import QtCore

Settings {
    category: "VIEWPRO"
    property bool showOSD: true
    // property int gimbalYawSpeed: 0
    // property int gimbalTiltSpeed: 0
    property bool trackingSW: false
    property int zoomSpeed: 4
    property bool autoFocus: true
    property bool digitalZoomEO: true
    property bool imageFlipEO: false
    property bool imageFlipIR: false
    // property bool colorBarIR: true
    // property bool nearIR: false
    property bool clickToTrack: false
    property bool trackingAI: false
    property bool autotrackAI: false
    // property bool imageEnhance: false
    // property bool defog: false
    property int irDZoom: 0
    property int irMode: 0
    property int trackingTemplate: 1
    property int videoSource: 0
    property bool trackingMode: false
}
