import QGroundControl.FlightDisplay

GuidedToolStripAction {
    text:       _guidedController.beaconTitle
    iconSource: "/InstrumentValueIcons/album.svg"
    visible:    _guidedController.showBeacon
    enabled:    _guidedController.showBeacon
    actionID:   _guidedController.actionBeacon
}
