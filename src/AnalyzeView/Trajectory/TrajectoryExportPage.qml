import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls

AnalyzePage {
    pageComponent: pageComponent
    pageDescription: qsTr("Export flight trajectory to OGC Moving Features JSON format. Supports live vehicle trajectory, CSV telemetry logs, and ULog files.")

    readonly property real _margin: ScreenTools.defaultFontPixelWidth * 2
    readonly property real _minWidth: ScreenTools.defaultFontPixelWidth * 20
    readonly property real _maxWidth: ScreenTools.defaultFontPixelWidth * 30

    TrajectoryExportController {
        id: exportController
    }

    Component {
        id: pageComponent

        GridLayout {
            columns: 2
            columnSpacing: _margin
            rowSpacing: ScreenTools.defaultFontPixelWidth * 2
            width: availableWidth

            // Source type selection
            QGCLabel {
                text: qsTr("Source Type")
                Layout.alignment: Qt.AlignVCenter
            }

            QGCComboBox {
                id: sourceTypeCombo
                Layout.fillWidth: true
                Layout.maximumWidth: _maxWidth
                model: [
                    qsTr("Live Vehicle Trajectory"),
                    qsTr("CSV Telemetry Log"),
                    qsTr("ULog File")
                ]
                currentIndex: exportController.sourceType
                onCurrentIndexChanged: exportController.sourceType = currentIndex
            }

            // Progress indicator
            BusyIndicator {
                running: exportController.inProgress
                width: progressBar.height
                height: progressBar.height
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            }

            ProgressBar {
                id: progressBar
                to: 100
                value: exportController.progress
                opacity: exportController.inProgress ? 1 : 0.25
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
            }

            // Error message
            QGCLabel {
                text: exportController.errorMessage
                color: "red"
                font.bold: true
                visible: exportController.errorMessage !== ""
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignHCenter
            }

            // Point count
            QGCLabel {
                text: qsTr("Points: %1").arg(exportController.pointCount)
                visible: exportController.pointCount > 0
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignHCenter
            }

            // Live trajectory info
            QGCLabel {
                text: qsTr("Active Vehicle: %1").arg(
                    QGroundControl.multiVehicleManager.activeVehicle
                        ? QGroundControl.multiVehicleManager.activeVehicle.id
                        : qsTr("None"))
                visible: sourceTypeCombo.currentIndex === 0
                Layout.columnSpan: 2
            }

            QGCLabel {
                text: qsTr("Trajectory Points: %1").arg(
                    QGroundControl.multiVehicleManager.activeVehicle
                        ? QGroundControl.multiVehicleManager.activeVehicle.trajectoryPoints.count
                        : 0)
                visible: sourceTypeCombo.currentIndex === 0
                Layout.columnSpan: 2
            }

            // Source file selection (for CSV/ULog)
            QGCButton {
                text: qsTr("Select Source File")
                visible: sourceTypeCombo.currentIndex > 0
                Layout.minimumWidth: _minWidth
                Layout.maximumWidth: _maxWidth
                Layout.fillWidth: true
                onClicked: sourceFileDialog.openForLoad()

                QGCFileDialog {
                    id: sourceFileDialog
                    title: qsTr("Select Source File")
                    nameFilters: sourceTypeCombo.currentIndex === 1
                        ? [qsTr("CSV Files (*.csv)"), qsTr("All Files (*)")]
                        : [qsTr("ULog Files (*.ulg)"), qsTr("All Files (*)")]
                    folder: QGroundControl.settingsManager.appSettings.telemetrySavePath
                    onAcceptedForLoad: (file) => {
                        exportController.sourceFile = file
                        close()
                    }
                }
            }

            QGCLabel {
                text: exportController.sourceFile || qsTr("No file selected")
                elide: Text.ElideLeft
                visible: sourceTypeCombo.currentIndex > 0
                Layout.fillWidth: true
            }

            // Output file selection
            QGCButton {
                text: qsTr("Select Output File")
                Layout.minimumWidth: _minWidth
                Layout.maximumWidth: _maxWidth
                Layout.fillWidth: true
                onClicked: outputFileDialog.openForSave()

                QGCFileDialog {
                    id: outputFileDialog
                    title: qsTr("Save Trajectory")
                    nameFilters: [qsTr("GeoJSON Files (*.geojson)"), qsTr("All Files (*)")]
                    defaultSuffix: "geojson"
                    onAcceptedForSave: (file) => {
                        exportController.outputFile = file
                        close()
                    }
                }
            }

            QGCLabel {
                text: exportController.outputFile || qsTr("No output file selected")
                elide: Text.ElideLeft
                Layout.fillWidth: true
            }

            // Export button
            RowLayout {
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignHCenter
                spacing: _margin

                QGCButton {
                    text: qsTr("Preview")
                    enabled: _canPreview && !exportController.inProgress
                    onClicked: {
                        var json = exportController.preview()
                        if (json) {
                            console.log("Preview:", json.substring(0, 500))
                        }
                    }

                    property bool _canPreview: {
                        if (sourceTypeCombo.currentIndex === 0) {
                            return false
                        }
                        return exportController.sourceFile !== ""
                    }
                }

                QGCButton {
                    text: exportController.inProgress ? qsTr("Cancel") : qsTr("Export")
                    enabled: _canExport || exportController.inProgress
                    onClicked: {
                        if (exportController.inProgress) {
                            exportController.cancel()
                        } else if (sourceTypeCombo.currentIndex === 0) {
                            exportController.exportLiveTrajectory(
                                QGroundControl.multiVehicleManager.activeVehicle)
                        } else {
                            exportController.exportToFile()
                        }
                    }

                    property bool _canExport: {
                        if (exportController.outputFile === "") {
                            return false
                        }
                        if (sourceTypeCombo.currentIndex === 0) {
                            return QGroundControl.multiVehicleManager.activeVehicle !== null
                        }
                        return exportController.sourceFile !== ""
                    }
                }
            }
        }
    }
}
