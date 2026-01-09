#include "GpxPlanDocument.h"
#include "MissionCommandTree.h"
#include "MissionCommandUIInfo.h"
#include "MissionItem.h"
#include "Vehicle.h"
#include "QmlObjectListModel.h"
#include "TransectStyleComplexItem.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QXmlStreamWriter>

namespace {
    constexpr const char *_gpxNamespace = "http://www.topografix.com/GPX/1/1";
    constexpr const char *_gpxSchemaLocation = "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd";
    constexpr double _polygonClosureThresholdMeters = 1.0;

    void writeGpxHeader(QXmlStreamWriter &xml)
    {
        xml.setAutoFormatting(true);
        xml.setAutoFormattingIndent(2);
        xml.writeStartDocument();
        xml.writeStartElement(QStringLiteral("gpx"));
        xml.writeAttribute(QStringLiteral("version"), QStringLiteral("1.1"));
        xml.writeAttribute(QStringLiteral("creator"), QCoreApplication::applicationName());
        xml.writeDefaultNamespace(QString::fromLatin1(_gpxNamespace));
        xml.writeAttribute(QStringLiteral("xmlns:xsi"), QStringLiteral("http://www.w3.org/2001/XMLSchema-instance"));
        xml.writeAttribute(QStringLiteral("xsi:schemaLocation"), QString::fromLatin1(_gpxSchemaLocation));
    }

    void writeWaypoint(QXmlStreamWriter &xml, const QGeoCoordinate &coord, const QString &name)
    {
        xml.writeStartElement(QStringLiteral("wpt"));
        xml.writeAttribute(QStringLiteral("lat"), QString::number(coord.latitude(), 'f', 8));
        xml.writeAttribute(QStringLiteral("lon"), QString::number(coord.longitude(), 'f', 8));

        if (!std::isnan(coord.altitude())) {
            xml.writeTextElement(QStringLiteral("ele"), QString::number(coord.altitude(), 'f', 2));
        }

        if (!name.isEmpty()) {
            xml.writeTextElement(QStringLiteral("name"), name);
        }

        xml.writeEndElement();
    }

    void writeRoutePoint(QXmlStreamWriter &xml, const QGeoCoordinate &coord)
    {
        xml.writeStartElement(QStringLiteral("rtept"));
        xml.writeAttribute(QStringLiteral("lat"), QString::number(coord.latitude(), 'f', 8));
        xml.writeAttribute(QStringLiteral("lon"), QString::number(coord.longitude(), 'f', 8));

        if (!std::isnan(coord.altitude())) {
            xml.writeTextElement(QStringLiteral("ele"), QString::number(coord.altitude(), 'f', 2));
        }

        xml.writeEndElement();
    }

    void writeRoute(QXmlStreamWriter &xml, const QList<QGeoCoordinate> &coords, const QString &name)
    {
        xml.writeStartElement(QStringLiteral("rte"));

        if (!name.isEmpty()) {
            xml.writeTextElement(QStringLiteral("name"), name);
        }

        for (const QGeoCoordinate &coord : coords) {
            writeRoutePoint(xml, coord);
        }

        xml.writeEndElement();
    }
}

void GpxPlanDocument::_addFlightPath(Vehicle* vehicle, QList<MissionItem*> rgMissionItems)
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

void GpxPlanDocument::_addWaypoints(Vehicle* vehicle, QList<MissionItem*> rgMissionItems)
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

void GpxPlanDocument::_addComplexItems(QmlObjectListModel* visualItems)
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

void GpxPlanDocument::addMission(Vehicle* vehicle, QmlObjectListModel* visualItems, QList<MissionItem*> rgMissionItems)
{
    _addFlightPath(vehicle, rgMissionItems);
    _addWaypoints(vehicle, rgMissionItems);
    _addComplexItems(visualItems);
}

bool GpxPlanDocument::saveToFile(const QString& filename, QString& errorString) const
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

    QXmlStreamWriter xml(&file);
    writeGpxHeader(xml);

    // Write waypoints
    int waypointIndex = 1;
    for (const QGeoCoordinate &coord : _waypoints) {
        writeWaypoint(xml, coord, QStringLiteral("WPT%1").arg(waypointIndex++, 3, 10, QLatin1Char('0')));
    }

    // Write flight path as route
    if (_flightPath.count() >= 2) {
        writeRoute(xml, _flightPath, QStringLiteral("Flight Path"));
    }

    // Write survey areas as closed routes
    int areaIndex = 1;
    for (const QList<QGeoCoordinate> &polygon : _surveyAreas) {
        if (polygon.count() >= 3) {
            QList<QGeoCoordinate> closedPolygon = polygon;
            if (closedPolygon.first().distanceTo(closedPolygon.last()) >= _polygonClosureThresholdMeters) {
                closedPolygon.append(closedPolygon.first());
            }
            writeRoute(xml, closedPolygon, QStringLiteral("Survey Area %1").arg(areaIndex++));
        }
    }

    xml.writeEndElement(); // gpx
    xml.writeEndDocument();

    return true;
}
