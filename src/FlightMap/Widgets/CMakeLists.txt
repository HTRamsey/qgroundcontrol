find_package(Qt6 REQUIRED COMPONENTS Qml)

add_custom_target(FlightMapWidgetsQml
	SOURCES
		CenterMapDropButton.qml
		CenterMapDropPanel.qml
        CompassDial.qml
        CompassHeadingIndicator.qml
		HorizontalCompassAttitude.qml
        IntegratedAttitudeIndicator.qml
        IntegratedCompassAttitude.qml
		MapFitFunctions.qml
		PhotoVideoControl.qml
		QGCArtificialHorizon.qml
		QGCAttitudeHUD.qml
		QGCAttitudeWidget.qml
		QGCCompassWidget.qml
		QGCPitchIndicator.qml
		QGCWaypointEditor.qml
		VerticalCompassAttitude.qml
)

