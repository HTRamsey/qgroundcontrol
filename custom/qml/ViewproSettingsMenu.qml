import QtQuick
import QtQuick.Controls

Menu {
    title: "Viewpro"
    Component.onCompleted: {
        let max = 0
        for (let i = 0; i < contentData.length; i++) {
            if(!!contentData[i]) {
                if(contentData[i].implicitWidth > max) {
                    max = contentData[i].implicitWidth
                }
            }
        }
        implicitWidth = max * 1.1
    }

    Menu {
        title: "Interface"
        Component.onCompleted: {
            let max = 0
            for (let i = 0; i < contentData.length; i++) {
                if(!!contentData[i]) {
                    if(contentData[i].implicitWidth > max) {
                        max = contentData[i].implicitWidth
                    }
                }
            }
            implicitWidth = max * 1.1
        }

        Menu {
            title: "Gimbal Mode"
            Component.onCompleted: {
                let max = 0
                for (let i = 0; i < contentData.length; i++) {
                    if(!!contentData[i]) {
                        if(contentData[i].implicitWidth > max) {
                            max = contentData[i].implicitWidth
                        }
                    }
                }
                implicitWidth = max * 1.1
            }
            enabled: ViewproData.linkUp

            Repeater {
                model: ["Recenter", "Follow Yaw", "Lock Yaw", "Look Down"]

                MenuItem {
                    text: modelData
                    required property string modelData
                    required property int index

                    onTriggered: {
                        switch (index) {
                            case 0:
                                Viewpro.gimbalHomeSerial()
                                break

                            case 1:
                                Viewpro.gimbalFollowYawSerial()
                                break

                            case 2:
                                Viewpro.gimbalFollowYawDisableSerial()
                                break

                            case 3:
                                Viewpro.gimbalLookDownSerial()
                                break

                            default:
                                break
                        }
                    }
                }
            }
        }

        Menu {
            title: "PIP Mode"
            Component.onCompleted: {
                let max = 0
                for (let i = 0; i < contentData.length; i++) {
                    if(!!contentData[i]) {
                        if(contentData[i].implicitWidth > max) {
                            max = contentData[i].implicitWidth
                        }
                    }
                }
                implicitWidth = max * 1.1
            }
            enabled: ViewproData.linkUp

            Repeater {
                model: ["EO/IR", "IR", "IR/EO", "EO"]

                MenuItem {
                    text: modelData
                    autoExclusive: true
                    checkable: true
                    checked: index === ViewproSettings.videoSource
                    required property int index
                    required property string modelData

                    onTriggered: {
                        ViewproSettings.videoSource = index
                        switch (index) {
                            case 0:
                                Viewpro.setVideoSourceEO1IRPIPSerial()
                                break

                            case 1:
                                Viewpro.setVideoSourceIRSerial()
                                break

                            case 2:
                                Viewpro.setVideoSourceIREO1PIPSerial()
                                break

                            case 3:
                                Viewpro.setVideoSourceEO1Serial()
                                break

                            default:
                                break
                        }
                    }
                }
            }
        }
    }

    Menu {
        title: "Tracking"
        Component.onCompleted: {
            let max = 0
            for (let i = 0; i < contentData.length; i++) {
                if(!!contentData[i]) {
                    if(contentData[i].implicitWidth > max) {
                        max = contentData[i].implicitWidth
                    }
                }
            }
            implicitWidth = max * 1.1
        }
        enabled: ViewproData.linkUp

        /*MenuItem {
            text:       "Vehicle Detection"
            checkable: true
            checked: ViewproSettings.trackingAI
            onTriggered: {
                Viewpro.trackingAISerial(checked);
                ViewproSettings.trackingAI = checked;
            }
        }

        MenuItem {
            text:       "Vehicle Autotrack"
            checkable: true
            checked: ViewproSettings.autotrackAI
            enabled: ViewproSettings.trackingAI

            onTriggered: {
                checked ? Viewpro.trackingAIAutotrackSerial() : _cammera.trackingSearchStopSerial();
                ViewproSettings.autotrackAI = checked;
            }
        }*/

        Menu {
            title: "Template Size"
            Component.onCompleted: {
                let max = 0
                for (let i = 0; i < contentData.length; i++) {
                    if(!!contentData[i]) {
                        if(contentData[i].implicitWidth > max) {
                            max = contentData[i].implicitWidth
                        }
                    }
                }
                implicitWidth = max * 1.1
            }

            Repeater {
                model: ["small", "medium", "big", "small-medium", "small-big", "medium-big", "small-medium-big"]

                MenuItem {
                    text: modelData
                    autoExclusive: true
                    checkable: true
                    checked: index === ViewproSettings.trackingTemplate
                    required property int index
                    required property string modelData

                    onTriggered: {
                        Viewpro.trackingTemplateSizeSerial(index)
                        ViewproSettings.trackingTemplate = index
                    }
                }
            }
        }

        /*MenuItem {
            text:       "Fast Tracking Speed"
            checkable: true
            checked: ViewproSettings.trackingMode

            onTriggered: {
                Viewpro.setTrackModeSerial(checked);
                ViewproSettings.trackingMode = checked;
            }
        }*/
    }

    Menu {
        title: "EO Stream"
        Component.onCompleted: {
            let max = 0
            for (let i = 0; i < contentData.length; i++) {
                if(!!contentData[i]) {
                    if(contentData[i].implicitWidth > max) {
                        max = contentData[i].implicitWidth
                    }
                }
            }
            implicitWidth = max * 1.1
        }
        enabled: ViewproData.linkUp

        MenuItem {
            text: "Auto Focus"
            checkable: true
            checked: ViewproSettings.autoFocus

            onTriggered: {
                Viewpro.setAutoFocusSerial(checked)
                ViewproSettings.autoFocus = checked
            }
        }

        MenuItem {
            text: "Digital Zoom"
            checkable: true
            checked: ViewproSettings.digitalZoomEO

            onTriggered: {
                Viewpro.setEODigitalZoomSerial(checked)
                ViewproSettings.digitalZoomEO = checked
            }
        }

        MenuItem {
            text: "Show On Screen Display"
            checkable: true
            checked: ViewproSettings.showOSD

            onTriggered: {
                Viewpro.setOSD1Serial(checked)
                Viewpro.setOSD2Serial(checked)
                ViewproSettings.showOSD = checked
            }
        }

        MenuItem {
            text: "Image Flip"
            checkable: true
            checked: ViewproSettings.imageFlipEO

            onTriggered: {
                // Viewpro.imageFlipEOSerial(checked);
                Viewpro.setFlipImageSerial(ViewproSettings.imageFlipIR, checked)
                ViewproSettings.imageFlipEO = checked
            }
        }

        Menu {
            title: "Zoom Speed"
            Component.onCompleted: {
                let max = 0
                for (let i = 0; i < contentData.length; i++) {
                    if(!!contentData[i]) {
                        if(contentData[i].implicitWidth > max) {
                            max = contentData[i].implicitWidth
                        }
                    }
                }
                implicitWidth = max * 1.1
            }

            Repeater {
                model: ["1", "2", "3", "4", "5", "6", "7"]

                MenuItem {
                    text: modelData
                    autoExclusive: true
                    checkable: true
                    checked: index === ViewproSettings.zoomSpeed
                    required property int index
                    required property string modelData

                    onTriggered: ViewproSettings.zoomSpeed = index
                }
            }
        }
    }

    Menu {
        title: "IR Stream"
        Component.onCompleted: {
            let max = 0
            for (let i = 0; i < contentData.length; i++) {
                if(!!contentData[i]) {
                    if(contentData[i].implicitWidth > max) {
                        max = contentData[i].implicitWidth
                    }
                }
            }
            implicitWidth = max * 1.1
        }
        enabled: ViewproData.linkUp

        Menu {
            title: "IR Mode"
            Component.onCompleted: {
                let max = 0
                for (let i = 0; i < contentData.length; i++) {
                    if(!!contentData[i]) {
                        if(contentData[i].implicitWidth > max) {
                            max = contentData[i].implicitWidth
                        }
                    }
                }
                implicitWidth = max * 1.1
            }

            Repeater {
                model: ["White Hot", "Black Hot", "Pseudo Color", "Ext1", "Ext2", "Ext3"]

                MenuItem {
                    text: modelData
                    autoExclusive: true
                    checkable: true
                    checked: index === ViewproSettings.irMode
                    required property int index
                    required property string modelData

                    onTriggered: {
                        ViewproSettings.irMode = index

                        switch (index) {
                            case 0:
                                Viewpro.setIRModeWhiteHotSerial()
                                break

                            case 1:
                                Viewpro.setIRModeBlackHotSerial()
                                break

                            case 2:
                                Viewpro.rainbowIRSerial()
                                break

                            case 3:
                            case 4:
                            case 5:
                            case 6:
                            case 7:
                            case 8:
                                Viewpro.setIRColorExtSerial(index + 1 - 3)
                                break

                            default:
                                break
                        }
                    }
                }
            }
        }

        Menu {
            title: "IR Digital Zoom"
            Component.onCompleted: {
                let max = 0
                for (let i = 0; i < contentData.length; i++) {
                    if(!!contentData[i]) {
                        if(contentData[i].implicitWidth > max) {
                            max = contentData[i].implicitWidth
                        }
                    }
                }
                implicitWidth = max * 1.1
            }

            Repeater {
                model: ["1x", "2x", "3x", "4x"]

                MenuItem {
                    text: modelData
                    autoExclusive: true
                    checkable: true
                    checked: index === ViewproSettings.irDZoom
                    required property int index
                    required property string modelData

                    onTriggered: {
                        Viewpro.zoomToIRSerial(index)
                        ViewproSettings.irDZoom = index
                    }
                }
            }
        }

        MenuItem {
            text: "Image Flip IR"
            checkable: true
            checked: ViewproSettings.imageFlipIR

            onTriggered: {
                Viewpro.imageFlipIRSerial(checked)
                ViewproSettings.imageFlipIR = checked
            }
        }
    }
}
