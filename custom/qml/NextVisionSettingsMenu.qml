import QtQuick
import QtQuick.Controls

Menu {
    title: "NextVision"
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
    }
}
