import QtQml.Models

import QGroundControl
import QGroundControl.Controls

ToolStripActionList {
    id: _root

    signal displayPreFlightChecklist

    model: [
        PreFlightCheckListShowAction { onTriggered: displayPreFlightChecklist() },
        GuidedActionTakeoff { },
        GuidedActionLand { },
        GuidedActionRTL { },
        GuidedActionAltitude { },
        GuidedActionPause { },
        GuidedActionSpotlight { },
        GuidedActionBeacon { },
        GuidedActionRemoteId { },
        GuidedActionNavigationLight { },
        GuidedActionAntiCollisionLight { }
    ]
}
