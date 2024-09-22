import QGroundControl.FlightDisplay

GuidedToolStripAction {
    text:       _guidedController.changeAltTitle
    iconSource: "/res/chevron-up.svg"
    visible:    _guidedController.showChangeAlt
    enabled:    _guidedController.showChangeAlt
    actionID:   _guidedController.actionChangeAlt
}
