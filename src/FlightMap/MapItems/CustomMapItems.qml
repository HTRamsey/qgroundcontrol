/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtLocation
import QtPositioning

import QGroundControl
import QGroundControl.Controls
import QGroundControl.FlightMap


// Allow custom builds to add visual items associated with the Flight Plan to the map
Item {
    property var    map             ///< Map control to show items on
    property bool   largeMapView    ///< true: map takes up entire view, false: map is in small window

    Instantiator {
        model: QGroundControl.corePlugin.customMapItems

        Item {
            property var _customObject

            Component.onCompleted: {
                var controlUrl = object.url
                if (controlUrl !== "") {
                    var component = Qt.createComponent(controlUrl);
                    if (component.status === Component.Ready) {
                        _customObject = component.createObject(map, { "customMapObject": object })
                        if (_customObject) {
                            map.addMapItem(_customObject)
                        }
                    } else {
                        console.log("Component creation failed", component.errorString())
                    }
                }
            }

            Component.onDestruction: {
                if (_customObject) {
                    _customObject.destroy()
                }
            }
        }
    }
}
