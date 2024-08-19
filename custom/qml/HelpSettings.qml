import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.Palette
import QGroundControl.ScreenTools

Rectangle {
    color:          qgcPal.window
    anchors.fill:   parent

    readonly property real _margins: ScreenTools.defaultFontPixelHeight

    QGCPalette { id: qgcPal; colorGroupEnabled: true }

    QGCFlickable {
        anchors.margins:    _margins
        anchors.fill:       parent
        contentWidth:       grid.width
        contentHeight:      grid.height
        clip:               true

        GridLayout {
            id:         grid
            columns:    2

            QGCLabel { text: qsTr("QGroundControl User Guide") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://docs.qgroundcontrol.com\">https://docs.qgroundcontrol.com</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("PX4 Users Discussion Forum") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"http://discuss.px4.io/c/qgroundcontrol\">http://discuss.px4.io/c/qgroundcontrol</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("ArduPilot Users Discussion Forum") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://discuss.ardupilot.org/c/ground-control-software/qgroundcontrol\">https://discuss.ardupilot.org/c/ground-control-software/qgroundcontrol</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("QGroundControl Discord Channel") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://discord.com/channels/1022170275984457759/1022185820683255908\">https://discord.com/channels/1022170275984457759/1022185820683255908</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("Zenith Aerotech Website") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://zenithaerotech.com\">https://zenithaerotech.com</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("FAA UAS General Information") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://www.faa.gov/uas\">https://www.faa.gov/uas</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("FAA Operator information") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://www.faa.gov/uas/commercial_operators\">https://www.faa.gov/uas/commercial_operators</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("FAA AIRSPACE Restrictions") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://www.faa.gov/uas/recreational_fliers/where_can_i_fly/airspace_restrictions\">https://www.faa.gov/uas/recreational_fliers/where_can_i_fly/airspace_restrictions</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("SkyVector Site for Restricted areas") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://skyvector.com\">https://skyvector.com</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("UAV Forecast") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://www.uavforecast.com\">https://www.uavforecast.com</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("Flight Radar") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://www.flightradar24.com\">https://www.flightradar24.com</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("Viewpro Camera") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"http://www.viewprotech.com/index.php?ac=article&at=read&did=442\">http://www.viewprotech.com/index.php?ac=article&at=read&did=442</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }

            QGCLabel { text: qsTr("NextVision Camera") }
            QGCLabel {
                linkColor:          qgcPal.text
                text:               "<a href=\"https://www.nextvision-sys.com/dragoneye-2\">https://www.nextvision-sys.com/dragoneye-2</a>"
                onLinkActivated:    (link) => Qt.openUrlExternally(link)
            }
        }
    }
}
