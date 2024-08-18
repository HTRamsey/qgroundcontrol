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
                        heading: "GPU Status"

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.psuTemperature.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.psuTemperature.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.spoolTemperature.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.spoolTemperature.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.psuFanDuty.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.psuFanDuty.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.spoolFanDuty.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.spoolFanDuty.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.motorDuty.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.motorDuty.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.tetherTension.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.tetherTension.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.latitude.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.latitude.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.longitude.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.longitude.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.altitude.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.altitude.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.pressure.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.pressure.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.pressureTemperature.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.pressureTemperature.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.outputVoltage.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.outputVoltage.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.psuTempWarning.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.psuTempWarning.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.psuTempCritical.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.psuTempCritical.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.spoolTempWarning.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.spoolTempWarning.valueString
                        }

                        LabelledLabel {
                            label:      QGroundControl.corePlugin.gpu.spoolTempCritical.shortDescription
                            labelText:  QGroundControl.corePlugin.gpu.spoolTempCritical.valueString
                        }
                    }
                }
            }
        }
    }
}
