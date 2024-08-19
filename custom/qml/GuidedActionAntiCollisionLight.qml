import QGroundControl.FlightDisplay

GuidedToolStripAction {
    text:       _guidedController.antiCollisionLightTitle
    iconSource: "/InstrumentValueIcons/flashlight.svg"
    visible:    _guidedController.showAntiCollisionLight
    enabled:    _guidedController.showAntiCollisionLight
    actionID:   _guidedController.actionAntiCollisionLight
}
