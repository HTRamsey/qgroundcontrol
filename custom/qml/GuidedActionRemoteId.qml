import QGroundControl.FlightDisplay

GuidedToolStripAction {
    text:       _guidedController.remoteIdTitle
    iconSource: "/InstrumentValueIcons/flashlight.svg"
    visible:    _guidedController.showRemoteId
    enabled:    _guidedController.showRemoteId
    actionID:   _guidedController.actionRemoteId
}
