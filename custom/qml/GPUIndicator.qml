import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.MultiVehicleManager
import QGroundControl.ScreenTools
import QGroundControl.Palette

Item {
    id:             control
    anchors.top:    parent.top
    anchors.bottom: parent.bottom
    width:          telemIcon.width * 1.1

    property bool showIndicator: _hasTelemetry

    property var  _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property bool _hasTelemetry: _activeVehicle

    QGCColoredImage {
        id:                 telemIcon
        anchors.top:        parent.top
        anchors.bottom:     parent.bottom
        width:              height
        sourceSize.height:  height
        source:             "/InstrumentValueIcons/portfolio.svg"
        fillMode:           Image.PreserveAspectFit
        color:              qgcPal.buttonText
    }

    MouseArea {
        anchors.fill:   parent
        onClicked:      mainWindow.showIndicatorDrawer(telemGPUInfoPage, control)
    }

    Component {
        id: telemGPUInfoPage

        ToolIndicatorPage {
            showExpand: false

            contentComponent: Component {
                ColumnLayout {
                    spacing: ScreenTools.defaultFontPixelHeight / 2

                    SettingsGroupLayout {
                        id: gpuGroup
                        heading: qsTr("GPU Status")
                        property var gpu: QGroundControl.corePlugin.gpu

                        SettingsGroupLayout {
                            heading: qsTr("Tether")

                            LabelledLabel {
                                label:      gpuGroup.gpu.tetherTension.shortDescription
                                labelText:  gpuGroup.gpu.tetherTension.valueString + " " + gpuGroup.gpu.tetherTension.units
                            }

                            LabelledLabel {
                                label:      gpuGroup.gpu.motorDuty.shortDescription
                                labelText:  gpuGroup.gpu.motorDuty.valueString + " " + gpuGroup.gpu.motorDuty.units
                            }
                        }

                        SettingsGroupLayout {
                            heading: qsTr("Power")

                            LabelledLabel {
                                label:      gpuGroup.gpu.outputVoltage.shortDescription
                                labelText:  gpuGroup.gpu.outputVoltage.valueString + " " + gpuGroup.gpu.outputVoltage.units
                            }
                        }

                        SettingsGroupLayout {
                            heading: qsTr("Temperature")

                            LabelledLabel {
                                label:      gpuGroup.gpu.psuTemperature.shortDescription
                                labelText:  gpuGroup.gpu.psuTemperature.valueString + " " + gpuGroup.gpu.psuTemperature.units
                            }

                            LabelledLabel {
                                label:      gpuGroup.gpu.spoolTemperature.shortDescription
                                labelText:  gpuGroup.gpu.spoolTemperature.valueString + " " + gpuGroup.gpu.spoolTemperature.units
                            }

                            LabelledLabel {
                                label:      gpuGroup.gpu.psuFanDuty.shortDescription
                                labelText:  gpuGroup.gpu.psuFanDuty.valueString + " " + gpuGroup.gpu.psuFanDuty.units
                            }

                            LabelledLabel {
                                label:      gpuGroup.gpu.spoolFanDuty.shortDescription
                                labelText:  gpuGroup.gpu.spoolFanDuty.valueString + " " + gpuGroup.gpu.spoolFanDuty.units
                            }
                        }

                        SettingsGroupLayout {
                            heading: qsTr("Location")

                            LabelledLabel {
                                label:      gpuGroup.gpu.latitude.shortDescription
                                labelText:  gpuGroup.gpu.latitude.valueString + " " + gpuGroup.gpu.latitude.units
                            }

                            LabelledLabel {
                                label:      gpuGroup.gpu.longitude.shortDescription
                                labelText:  gpuGroup.gpu.longitude.valueString + " " + gpuGroup.gpu.longitude.units
                            }

                            LabelledLabel {
                                label:      gpuGroup.gpu.altitude.shortDescription
                                labelText:  gpuGroup.gpu.altitude.valueString + " " + gpuGroup.gpu.altitude.units
                            }
                        }

                        SettingsGroupLayout {
                            heading: qsTr("Pressure")

                            LabelledLabel {
                                label:      gpuGroup.gpu.pressure.shortDescription
                                labelText:  gpuGroup.gpu.pressure.valueString + " " + gpuGroup.gpu.pressure.units
                            }

                            LabelledLabel {
                                label:      gpuGroup.gpu.pressureTemperature.shortDescription
                                labelText:  gpuGroup.gpu.pressureTemperature.valueString + " " + gpuGroup.gpu.pressureTemperature.units
                            }
                        }

                        SettingsGroupLayout {
                            heading: qsTr("Warnings")

                            LabelledLabel {
                                label:      gpuGroup.gpu.psuTempWarning.shortDescription
                                labelText:  gpuGroup.gpu.psuTempWarning.valueString + " " + gpuGroup.gpu.psuTempWarning.units
                            }

                            LabelledLabel {
                                label:      gpuGroup.gpu.psuTempCritical.shortDescription
                                labelText:  gpuGroup.gpu.psuTempCritical.valueString + " " + gpuGroup.gpu.psuTempCritical.units
                            }

                            LabelledLabel {
                                label:      gpuGroup.gpu.spoolTempWarning.shortDescription
                                labelText:  gpuGroup.gpu.spoolTempWarning.valueString + " " + gpuGroup.gpu.spoolTempWarning.units
                            }

                            LabelledLabel {
                                label:      gpuGroup.gpu.spoolTempCritical.shortDescription
                                labelText:  gpuGroup.gpu.spoolTempCritical.valueString + " " + gpuGroup.gpu.spoolTempCritical.units
                            }
                        }
                    }
                }
            }
        }
    }
}
