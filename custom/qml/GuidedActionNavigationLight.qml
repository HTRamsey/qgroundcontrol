import QGroundControl.FlightDisplay

GuidedToolStripAction {
    text:       _guidedController.navigationLightTitle
    iconSource: "/InstrumentValueIcons/flashlight.svg"
    visible:    _guidedController.showNavigationLight
    enabled:    _guidedController.showNavigationLight
    actionID:   _guidedController.actionNavigationLight
}
