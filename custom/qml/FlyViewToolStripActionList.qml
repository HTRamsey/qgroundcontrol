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
        GuidedActionPause { },
        GuidedActionActionList { },
        GuidedActionSpotlight { },
        GuidedActionBeacon { },
        GuidedActionRemoteId { },
        GuidedActionNavigationLight { },
        GuidedActionAntiCollisionLight { }
    ]
}
