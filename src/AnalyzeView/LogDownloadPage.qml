/****************************************************************************
 *
 * (c) 2009-2025 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels

import QGroundControl
import QGroundControl.Controls
import QGroundControl.ScreenTools

AnalyzePage {
    id: logDownloadPage

    // Page metadata
    pageComponent: pageComponent
    pageDescription: qsTr("Log Download allows you to download binary log files from your vehicle. Click Refresh to get list of available logs.")

    // Convenience accessors / derived state
    readonly property var  ctl: LogDownloadController
    readonly property bool isBusy: ctl.requestingList || ctl.downloadingLogs
    readonly property int  rowCount: ctl.model.count + 1

    Component {
        id: pageComponent

        RowLayout {
            width: availableWidth
            height: availableHeight

            // List/grid
            QGCFlickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: gridLayout.implicitWidth
                contentHeight: gridLayout.implicitHeight

                GridLayout {
                    id: gridLayout

                    rows: rowCount
                    columns: 5
                    flow: GridLayout.TopToBottom
                    columnSpacing: ScreenTools.defaultFontPixelWidth
                    rowSpacing: 0

                    // --- Column: Select ---
                    QGCCheckBox {
                        id: headerCheckBox
                        enabled: false
                    }
                    Repeater {
                        model: ctl.model
                        QGCCheckBox {
                            checked: object.selected
                            onToggled: object.selected = checked
                        }
                    }

                    // --- Column: Id ---
                    QGCLabel { text: qsTr("Id") }
                    Repeater {
                        model: ctl.model
                        QGCLabel { text: object.id }
                    }

                    // --- Column: Date ---
                    QGCLabel { text: qsTr("Date") }
                    Repeater {
                        model: ctl.model
                        QGCLabel {
                            text: {
                                if (!object.received) return ""
                                if (object.time.getUTCFullYear() < 2010) return qsTr("Date Unknown")
                                // Default-locale short format
                                return Qt.formatDateTime(object.time, Qt.DefaultLocaleShortDate)
                            }
                        }
                    }

                    // --- Column: Size ---
                    QGCLabel { text: qsTr("Size") }
                    Repeater {
                        model: ctl.model
                        QGCLabel { text: object.sizeStr }
                    }

                    // --- Column: Status ---
                    QGCLabel { text: qsTr("Status") }
                    Repeater {
                        model: ctl.model
                        QGCLabel { text: object.status }
                    }
                }
            }

            // Actions
            ColumnLayout {
                spacing: ScreenTools.defaultFontPixelWidth
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: false

                // Refresh
                QGCButton {
                    Layout.fillWidth: true
                    text: qsTr("Refresh")
                    enabled: !isBusy

                    onClicked: {
                        const v = QGroundControl.multiVehicleManager.activeVehicle
                        if (!v || v.isOfflineEditingVehicle) {
                            mainWindow.showMessageDialog(
                                qsTr("Log Refresh"),
                                qsTr("You must be connected to a vehicle in order to download logs.")
                            )
                            return
                        }
                        ctl.refresh()
                    }
                }

                // Download
                QGCButton {
                    Layout.fillWidth: true
                    text: qsTr("Download")
                    enabled: !isBusy

                    onClicked: {
                        // any selection?
                        let hasSelection = false
                        for (let i = 0; i < ctl.model.count; i++) {
                            if (ctl.model.get(i).selected) { hasSelection = true; break }
                        }

                        if (!hasSelection) {
                            mainWindow.showMessageDialog(
                                qsTr("Log Download"),
                                qsTr("You must select at least one log file to download.")
                            )
                            return
                        }

                        if (ScreenTools.isMobile) {
                            ctl.download()
                            return
                        }

                        fileDialog.title = qsTr("Select save directory")
                        fileDialog.folder = QGroundControl.settingsManager.appSettings.logSavePath
                        fileDialog.selectFolder = true
                        fileDialog.openForLoad()
                    }

                    QGCFileDialog {
                        id: fileDialog
                        onAcceptedForLoad: (dir) => {
                            ctl.download(dir)
                            close()
                        }
                    }
                }

                // Erase All
                QGCButton {
                    Layout.fillWidth: true
                    text: qsTr("Erase All")
                    enabled: !isBusy && (ctl.model.count > 0)
                    onClicked: mainWindow.showMessageDialog(
                        qsTr("Delete All Log Files"),
                        qsTr("All log files will be erased permanently. Is this really what you want?"),
                        Dialog.Yes | Dialog.No,
                        function() { ctl.eraseAll() }
                    )
                }

                // Cancel
                QGCButton {
                    Layout.fillWidth: true
                    text: qsTr("Cancel")
                    enabled: isBusy
                    onClicked: ctl.cancel()
                }
            }
        }
    }
}
