/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/



import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import QGroundControl

import QGroundControl.FactControls
import QGroundControl.Controls
import QGroundControl.ScreenTools


SetupPage {
    id:             syslinkPage
    pageComponent:  pageComponent

    Component {
        id: pageComponent

        Column {
            id:         innerColumn
            width:      availableWidth
            spacing:    ScreenTools.defaultFontPixelHeight * 0.5

            property int textEditWidth:    ScreenTools.defaultFontPixelWidth * 12


            SyslinkComponentController {
                id:         controller
            }

            QGCLabel {
                text: qsTr("Radio Settings")
                font.bold:   true
            }

            Rectangle {
                width:  parent.width
                height: radioGrid.height + ScreenTools.defaultFontPixelHeight
                color:  qgcPal.windowShade

                GridLayout {
                    id:                 radioGrid
                    anchors.margins:    ScreenTools.defaultFontPixelHeight / 2
                    anchors.left:       parent.left
                    anchors.top:        parent.top
                    columns:            2
                    columnSpacing:      ScreenTools.defaultFontPixelWidth

                    QGCLabel {
                        text:               qsTr("Channel")
                    }

                    QGCTextField {
                        id:                     channelField
                        width:                  textEditWidth
                        text:                   controller.radioChannel
                        validator:              IntValidator {bottom: 0; top: 125;}
                        inputMethodHints:       Qt.ImhDigitsOnly
                        onEditingFinished: {
                            controller.radioChannel = text
                        }
                    }

                    QGCLabel {
                        id:                 channelHelp
                        Layout.columnSpan:  radioGrid.columns
                        Layout.fillWidth:   true
                        font.pointSize:     ScreenTools.smallFontPointSize
                        wrapMode:           Text.WordWrap
                        text:               "Channel can be between 0 and 125"
                    }

                    QGCLabel {
                        id:                 addressLabel
                        text:               qsTr("Address")
                    }

                    QGCTextField {
                        id:                     addressField
                        width:                  textEditWidth
                        text:                   controller.radioAddress
                        maximumLength:          10
                        validator:              RegExpValidator { regExp: /^[0-9A-Fa-f]*$/ }
                        onEditingFinished: {
                            controller.radioAddress = text
                        }
                    }

                    QGCLabel {
                        id:                 addressHelp
                        Layout.columnSpan:  radioGrid.columns
                        Layout.fillWidth:   true
                        font.pointSize:     ScreenTools.smallFontPointSize
                        wrapMode:           Text.WordWrap
                        text:               qsTr("Address in hex. Default is E7E7E7E7E7.")
                    }


                    QGCLabel {
                        id:                 rateLabel
                        text:               qsTr("Data Rate")
                    }

                    QGCComboBox {
                        id:                     rateField
                        Layout.fillWidth:       true
                        model:                  controller.radioRates
                        currentIndex:           controller.radioRate
                        onActivated: (index) => {
                            controller.radioRate = index
                        }
                    }

                    QGCButton {
                        text:                           qsTr("Restore Defaults")
                        width:                          textEditWidth
                        onClicked: {
                            controller.resetDefaults()
                        }
                    }

                } // Grid
            } // Rectangle - Radio Settings


        }
    }
}
