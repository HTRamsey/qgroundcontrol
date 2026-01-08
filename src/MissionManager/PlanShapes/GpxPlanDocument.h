#pragma once

#include <QtCore/QList>
#include <QtPositioning/QGeoCoordinate>

class MissionItem;
class Vehicle;
class QmlObjectListModel;

/// Used to convert a Plan to a GPX document
/// GPX 1.1 Specification: https://www.topografix.com/GPX/1/1/
class GpxPlanDocument
{
public:
    GpxPlanDocument();

    void addMission(Vehicle* vehicle, QmlObjectListModel* visualItems, QList<MissionItem*> rgMissionItems);

    bool saveToFile(const QString& filename, QString& errorString) const;

private:
    void _addFlightPath(Vehicle* vehicle, QList<MissionItem*> rgMissionItems);
    void _addWaypoints(Vehicle* vehicle, QList<MissionItem*> rgMissionItems);
    void _addComplexItems(QmlObjectListModel* visualItems);

    QList<QGeoCoordinate> _flightPath;
    QList<QGeoCoordinate> _waypoints;
    QList<QList<QGeoCoordinate>> _surveyAreas;
};
