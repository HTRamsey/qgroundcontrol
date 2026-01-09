#include "PlanDocument.h"

void PlanDocumentBase::addMission(Vehicle* vehicle, QmlObjectListModel* visualItems, QList<MissionItem*> rgMissionItems)
{
    Q_UNUSED(vehicle)
    Q_UNUSED(visualItems)
    Q_UNUSED(rgMissionItems)
    // Default implementation just clears - subclasses override to extract data
    clear();
}

void PlanDocumentBase::clear()
{
    _flightPath.clear();
    _waypoints.clear();
    _surveyAreas.clear();
    _surveyAreaNames.clear();
}

bool PlanDocumentBase::isEmpty() const
{
    return _flightPath.isEmpty() && _waypoints.isEmpty() && _surveyAreas.isEmpty();
}
