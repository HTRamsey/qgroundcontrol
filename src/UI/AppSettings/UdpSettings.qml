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
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.ScreenTools
import QGroundControl.Palette

ColumnLayout {
    spacing: _rowSpacing

    property var _linkManager: QGroundControl.linkManager
    property string selectedEntry: ""

    function saveSettings() {
        if (selectedEntry !== "") {
            const parts = selectedEntry.split(":")
            const ip = _linkManager.getIpAddress(parts[0])
            if (ip !== "") {
                hostField.text = ip + ":" + parts[1]
            }
        }
    }

    QGCLabel {
        Layout.preferredWidth: _secondColumnWidth
        Layout.fillWidth:       true
        font.pointSize:         ScreenTools.smallFontPointSize
        wrapMode:               Text.WordWrap
        text:                   qsTr("Note: For best perfomance, please disable AutoConnect to UDP devices on the General page.")
    }

    RowLayout {
        spacing: _colSpacing

        QGCLabel { text: qsTr("Port") }
        QGCTextField {
            id:                     portField
            text:                   subEditConfig.localPort.toString()
            focus:                  true
            Layout.preferredWidth:  _secondColumnWidth
            inputMethodHints:       Qt.ImhFormattedNumbersOnly
            onTextChanged:          subEditConfig.localPort = parseInt(portField.text)
        }
    }

    QGCLabel { text: qsTr("Server Addresses (optional)") }

    Repeater {
        model: subEditConfig.hostList

        delegate: RowLayout {
            spacing: _colSpacing

            QGCButton {
                text: modelData
                onClicked: {
                    selectedEntry = modelData
                    const parts = modelData.split(":")
                    const hostname = parts[0]
                    const port = parseInt(parts[1])
                    const resolved = _linkManager.getIpAddress(hostname)
                    if (resolved !== "") {
                        hostField.text = resolved + ":" + port
                    } else {
                        hostField.text = hostname + ":" + port
                    }
                }
            }

            QGCButton {
                text: qsTr("Remove")
                onClicked: {
                    subEditConfig.removeHost(modelData)
                    if (selectedEntry === modelData) {
                        selectedEntry = ""
                    }
                }
            }
        }
    }

    RowLayout {
        spacing: _colSpacing

        QGCTextField {
            id:                     hostField
            Layout.preferredWidth:  _secondColumnWidth
            placeholderText:        qsTr("Example: 127.0.0.1:14550")
        }
        QGCButton {
            text:       qsTr("Add Server")
            enabled:    hostField.text !== ""
            onClicked: {
                subEditConfig.addHost(hostField.text)
                hostField.text = ""
            }
        }
    }
}
