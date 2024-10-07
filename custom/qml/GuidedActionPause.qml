import QGroundControl.FlightDisplay

GuidedToolStripAction {
    text:       _guidedController.pauseTitle
    iconSource: "/res/pause-mission.svg"
    visible:    _guidedController.showPause
    enabled:    _guidedController.showPause
    actionID:   _guidedController.actionPause
}
