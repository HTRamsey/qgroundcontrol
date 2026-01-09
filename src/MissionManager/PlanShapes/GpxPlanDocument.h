#pragma once

#include "PlanDocument.h"

#include <QtCore/QList>
#include <QtPositioning/QGeoCoordinate>

class MissionItem;
class Vehicle;
class QmlObjectListModel;

/// Used to convert a Plan to a GPX document
/// GPX 1.1 Specification: https://www.topografix.com/GPX/1/1/
class GpxPlanDocument : public PlanDocumentBase
{
public:
    GpxPlanDocument() = default;

    void addMission(Vehicle* vehicle, QmlObjectListModel* visualItems, QList<MissionItem*> rgMissionItems) override;

    bool saveToFile(const QString& filename, QString& errorString) const;

private:
    void _addFlightPath(Vehicle* vehicle, QList<MissionItem*> rgMissionItems);
    void _addWaypoints(Vehicle* vehicle, QList<MissionItem*> rgMissionItems);
    void _addComplexItems(QmlObjectListModel* visualItems);
};
