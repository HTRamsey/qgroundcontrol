import QGroundControl.FlightDisplay

GuidedToolStripAction {
    text:       _guidedController.spotlightTitle
    iconSource: "/InstrumentValueIcons/flashlight.svg"
    visible:    _guidedController.showSpotlight
    enabled:    _guidedController.showSpotlight
    actionID:   _guidedController.actionSpotlight
}
