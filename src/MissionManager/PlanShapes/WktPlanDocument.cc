#include "WktPlanDocument.h"
#include "WKTHelper.h"
#include "MissionCommandTree.h"
#include "MissionCommandUIInfo.h"
#include "MissionItem.h"
#include "Vehicle.h"
#include "QmlObjectListModel.h"
#include "TransectStyleComplexItem.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>

void WktPlanDocument::_addFlightPath(Vehicle* vehicle, QList<MissionItem*> rgMissionItems)
{
    if (rgMissionItems.count() == 0) {
        return;
    }

    QGeoCoordinate homeCoord = rgMissionItems[0]->coordinate();

    for (const MissionItem* item : rgMissionItems) {
        const MissionCommandUIInfo* uiInfo = MissionCommandTree::instance()->getUIInfo(vehicle, QGCMAVLink::VehicleClassGeneric, item->command());
        if (uiInfo) {
            double altAdjustment = item->frame() == MAV_FRAME_GLOBAL ? 0 : homeCoord.altitude();

            if (uiInfo->isTakeoffCommand() && !vehicle->fixedWing()) {
                QGeoCoordinate coord = homeCoord;
                coord.setAltitude(item->param7() + altAdjustment);
                _flightPath.append(coord);
            }

            if (uiInfo->specifiesCoordinate() && !uiInfo->isStandaloneCoordinate()) {
                QGeoCoordinate coord = item->coordinate();
                coord.setAltitude(coord.altitude() + altAdjustment);
                _flightPath.append(coord);
            }
        }
    }
}

void WktPlanDocument::_addWaypoints(Vehicle* vehicle, QList<MissionItem*> rgMissionItems)
{
    if (rgMissionItems.count() == 0) {
        return;
    }

    QGeoCoordinate homeCoord = rgMissionItems[0]->coordinate();

    for (const MissionItem* item : rgMissionItems) {
        const MissionCommandUIInfo* uiInfo = MissionCommandTree::instance()->getUIInfo(vehicle, QGCMAVLink::VehicleClassGeneric, item->command());
        if (uiInfo && uiInfo->specifiesCoordinate()) {
            double altAdjustment = item->frame() == MAV_FRAME_GLOBAL ? 0 : homeCoord.altitude();
            QGeoCoordinate coord = item->coordinate();
            coord.setAltitude(coord.altitude() + altAdjustment);
            _waypoints.append(coord);
        }
    }
}

void WktPlanDocument::_addComplexItems(QmlObjectListModel* visualItems)
{
    for (int i = 0; i < visualItems->count(); i++) {
        auto* transectItem = visualItems->value<TransectStyleComplexItem*>(i);
        if (transectItem) {
            QGCMapPolygon* polygon = transectItem->surveyAreaPolygon();
            if (polygon && polygon->count() >= 3) {
                QList<QGeoCoordinate> vertices;
                for (int j = 0; j < polygon->count(); j++) {
                    vertices.append(polygon->vertexCoordinate(j));
                }
                _surveyAreas.append(vertices);
            }
        }
    }
}

void WktPlanDocument::addMission(Vehicle* vehicle, QmlObjectListModel* visualItems, QList<MissionItem*> rgMissionItems)
{
    _addFlightPath(vehicle, rgMissionItems);
    _addWaypoints(vehicle, rgMissionItems);
    _addComplexItems(visualItems);
}

QString WktPlanDocument::flightPathWkt() const
{
    if (_flightPath.count() < 2) {
        return QString();
    }
    return WKTHelper::toWKTLineString(_flightPath, _includeAltitude);
}

QString WktPlanDocument::waypointsWkt() const
{
    if (_waypoints.isEmpty()) {
        return QString();
    }
    return WKTHelper::toWKTMultiPoint(_waypoints, _includeAltitude);
}

QString WktPlanDocument::surveyAreasWkt() const
{
    if (_surveyAreas.isEmpty()) {
        return QString();
    }
    return WKTHelper::toWKTMultiPolygon(_surveyAreas, _includeAltitude);
}

QString WktPlanDocument::combinedWkt() const
{
    QStringList geometries;

    QString path = flightPathWkt();
    if (!path.isEmpty()) {
        geometries.append(path);
    }

    QString waypoints = waypointsWkt();
    if (!waypoints.isEmpty()) {
        geometries.append(waypoints);
    }

    QString areas = surveyAreasWkt();
    if (!areas.isEmpty()) {
        geometries.append(areas);
    }

    if (geometries.isEmpty()) {
        return QStringLiteral("GEOMETRYCOLLECTION EMPTY");
    }

    return QStringLiteral("GEOMETRYCOLLECTION (%1)").arg(geometries.join(QStringLiteral(", ")));
}

bool WktPlanDocument::saveToFile(const QString& filename, QString& errorString) const
{
    if (_waypoints.isEmpty() && _flightPath.isEmpty() && _surveyAreas.isEmpty()) {
        errorString = QObject::tr("No data to export");
        return false;
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorString = QObject::tr("Cannot open file for writing: %1").arg(file.errorString());
        return false;
    }

    QTextStream stream(&file);

    // Write each geometry type on its own line with a label
    QString path = flightPathWkt();
    if (!path.isEmpty()) {
        stream << "# Flight Path\n" << path << "\n\n";
    }

    QString waypoints = waypointsWkt();
    if (!waypoints.isEmpty()) {
        stream << "# Waypoints\n" << waypoints << "\n\n";
    }

    QString areas = surveyAreasWkt();
    if (!areas.isEmpty()) {
        stream << "# Survey Areas\n" << areas << "\n\n";
    }

    // Also write combined geometry collection
    stream << "# Combined (GEOMETRYCOLLECTION)\n" << combinedWkt() << "\n";

    return true;
}
