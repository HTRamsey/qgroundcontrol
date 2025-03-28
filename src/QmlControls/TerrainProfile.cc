/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "TerrainProfile.h"
#include "ComplexMissionItem.h"
#include "FlightPathSegment.h"
#include "MissionController.h"
#include "QGCApplication.h"
#include "QGCLoggingCategory.h"
#include "QmlObjectListModel.h"

#include <QtQuick/QSGFlatColorMaterial>
#include <QtQuick/QSGGeometryNode>

QGC_LOGGING_CATEGORY(TerrainProfileLog, "qgc.qmlcontrols.terrainprofile")

TerrainProfile::TerrainProfile(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);

    (void) connect(this, &QQuickItem::heightChanged, this, &QQuickItem::update);
    (void) connect(this, &TerrainProfile::visibleWidthChanged, this, &QQuickItem::update);

    // This collapse multiple updateSignals in a row to a single update
    (void) connect(this, &TerrainProfile::updateSignal, this, &QQuickItem::update, Qt::QueuedConnection);
    qgcApp()->addCompressedSignal(QMetaMethod::fromSignal(&TerrainProfile::updateSignal));
}

void TerrainProfile::componentComplete()
{
    QQuickItem::componentComplete();
}

void TerrainProfile::setMissionController(MissionController *missionController)
{
    if (missionController != _missionController) {
        _missionController = missionController;
        _visualItems = _missionController->visualItems();

        emit missionControllerChanged();

        (void) connect(_missionController, &MissionController::visualItemsChanged, this, &TerrainProfile::_newVisualItems);
        (void) connect(this, &TerrainProfile::visibleWidthChanged, this, &TerrainProfile::updateSignal, Qt::QueuedConnection);
        (void) connect(_missionController, &MissionController::recalcTerrainProfile, this, &TerrainProfile::updateSignal, Qt::QueuedConnection);
    }
}

void TerrainProfile::_newVisualItems()
{
    _visualItems = _missionController->visualItems();
    emit updateSignal();
}

void TerrainProfile::_createGeometry(QSGGeometryNode *&geometryNode, QSGGeometry *&geometry, QSGGeometry::DrawingMode drawingMode, const QColor &color)
{
    QSGFlatColorMaterial *const terrainMaterial = new QSGFlatColorMaterial();
    terrainMaterial->setColor(color);

    geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
    geometry->setDrawingMode(drawingMode);
    geometry->setLineWidth(2);

    geometryNode = new QSGGeometryNode();
    geometryNode->setFlag(QSGNode::OwnsGeometry);
    geometryNode->setFlag(QSGNode::OwnsMaterial);
    geometryNode->setFlag(QSGNode::OwnedByParent);
    geometryNode->setMaterial(terrainMaterial);
    geometryNode->setGeometry(geometry);
}

void TerrainProfile::_updateSegmentCounts(const FlightPathSegment *segment, int &cFlightProfileSegments, int &cTerrainProfilePoints, int &cMissingTerrainSegments, int &cTerrainCollisionSegments, double &minTerrainHeight, double &maxTerrainHeight)
{
    if (_shouldAddFlightProfileSegment(segment)) {
        if (segment->segmentType() == FlightPathSegment::SegmentTypeTerrainFrame) {
            // We show a full above terrain profile for flight segment
            cFlightProfileSegments += segment->amslTerrainHeights().count() - 1;
        } else {
            cFlightProfileSegments++;
        }
    }

    if (_shouldAddMissingTerrainSegment(segment)) {
        cMissingTerrainSegments += 1;
    } else {
        cTerrainProfilePoints += segment->amslTerrainHeights().count();
        for (int i = 0; i < segment->amslTerrainHeights().count(); i++) {
            minTerrainHeight = std::fmin(minTerrainHeight, segment->amslTerrainHeights()[i].value<double>());
            maxTerrainHeight = std::fmax(maxTerrainHeight, segment->amslTerrainHeights()[i].value<double>());
        }
    }

    if (segment->terrainCollision()) {
        cTerrainCollisionSegments++;
    }
}

void TerrainProfile::_addTerrainProfileSegment(const FlightPathSegment *segment, double currentDistance, double amslAltRange, QSGGeometry::Point2D *terrainVertices, int &terrainProfileVertexIndex) const
{
    double terrainDistance = 0;
    for (int heightIndex = 0; heightIndex < segment->amslTerrainHeights().count(); heightIndex++) {
        // Move along the x axis which is distance
        if (heightIndex == 0) {
            // The first point in the segment is at the position of the last point. So nothing to do here.
        } else if (heightIndex == segment->amslTerrainHeights().count() - 2) {
            // The distance between the last two heights differs with each terrain query
            terrainDistance += segment->finalDistanceBetween();
        } else {
            // The distance between all terrain heights except for the last is the same
            terrainDistance += segment->distanceBetween();
        }

        // Move along the y axis which is a view or terrain height as a percentage between the min/max AMSL altitude for all segments
        const double amslTerrainHeight = segment->amslTerrainHeights()[heightIndex].value<double>();
        const double terrainHeightPercent = (amslTerrainHeight - _minAMSLAlt) / amslAltRange;

        const float x = (currentDistance + terrainDistance) * _pixelsPerMeter;
        const float y = height() - (terrainHeightPercent * height());
        terrainVertices[terrainProfileVertexIndex++].set(x, y);
    }
}

void TerrainProfile::_addMissingTerrainSegment(const FlightPathSegment *segment, double currentDistance, QSGGeometry::Point2D *missingTerrainVertices, int &missingterrainProfileVertexIndex) const
{
    if (_shouldAddMissingTerrainSegment(segment)) {
        const float x = currentDistance * _pixelsPerMeter;
        const float y = height();
        missingTerrainVertices[missingterrainProfileVertexIndex++].set(x, y);
        missingTerrainVertices[missingterrainProfileVertexIndex++].set(x + (segment->totalDistance() * _pixelsPerMeter), y);
    }
}

void TerrainProfile::_addTerrainCollisionSegment(const FlightPathSegment *segment, double currentDistance, double amslAltRange, QSGGeometry::Point2D *terrainCollisionVertices, int &terrainCollisionVertexIndex) const
{
    if (segment->terrainCollision()) {
        const double amslCoord1Height = segment->coord1AMSLAlt();
        const double amslCoord2Height = segment->coord2AMSLAlt();
        const double coord1HeightPercent = (amslCoord1Height - _minAMSLAlt) / amslAltRange;
        const double coord2HeightPercent = (amslCoord2Height - _minAMSLAlt) / amslAltRange;

        float x = currentDistance * _pixelsPerMeter;
        float y = height() - (coord1HeightPercent * height());

        terrainCollisionVertices[terrainCollisionVertexIndex++].set(x, y);

        x += segment->totalDistance() * _pixelsPerMeter;
        y = height() - (coord2HeightPercent * height());

        terrainCollisionVertices[terrainCollisionVertexIndex++].set(x, y);
    }
}

void TerrainProfile::_addFlightProfileSegment(const FlightPathSegment *segment, double currentDistance, double amslAltRange, QSGGeometry::Point2D *flightProfileVertices, int &flightProfileVertexIndex) const
{
    if (!_shouldAddFlightProfileSegment(segment)) {
        return;
    }

    if (segment->segmentType() == FlightPathSegment::SegmentTypeTerrainFrame) {
        double terrainDistance = 0;
        const double distanceToSurface = segment->coord1AMSLAlt() - segment->amslTerrainHeights().first().value<double>();
        for (int heightIndex = 0; heightIndex < segment->amslTerrainHeights().count(); heightIndex++) {
            // Move along the x axis which is distance
            if (heightIndex == 0) {
                // The first point in the segment is at the position of the last point. So nothing to do here.
            } else if (heightIndex == (segment->amslTerrainHeights().count() - 2)) {
                // The distance between the last two heights differs with each terrain query
                terrainDistance += segment->finalDistanceBetween();
            } else {
                // The distance between all terrain heights except for the last is the same
                terrainDistance += segment->distanceBetween();
            }

            if (heightIndex > 1) {
                // Add first coord of segment
                const QSGGeometry::Point2D previousVertex = flightProfileVertices[flightProfileVertexIndex - 1];
                flightProfileVertices[flightProfileVertexIndex++].set(previousVertex.x, previousVertex.y);
            }

            // Add second coord of segment (or very first one)
            const double amslTerrainHeight = segment->amslTerrainHeights()[heightIndex].value<double>() + distanceToSurface;
            const double terrainHeightPercent = (amslTerrainHeight - _minAMSLAlt) / amslAltRange;

            const float x = (currentDistance + terrainDistance) * _pixelsPerMeter;
            const float y = height() - (terrainHeightPercent * height());
            flightProfileVertices[flightProfileVertexIndex++].set(x, y);
        }
    } else {
        const double amslCoord1Height = segment->coord1AMSLAlt();
        const double amslCoord2Height = segment->coord2AMSLAlt();
        const double coord1HeightPercent = (amslCoord1Height - _minAMSLAlt) / amslAltRange;
        const double coord2HeightPercent = (amslCoord2Height - _minAMSLAlt) / amslAltRange;

        float x = currentDistance * _pixelsPerMeter;
        float y = height() - (coord1HeightPercent * height());

        flightProfileVertices[flightProfileVertexIndex++].set(x, y);

        x += segment->totalDistance() * _pixelsPerMeter;
        y = height() - (coord2HeightPercent * height());

        flightProfileVertices[flightProfileVertexIndex++].set(x, y);
    }
}

QSGNode *TerrainProfile::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData */*updatePaintNodeData*/)
{
    // First we need to determine:
    //  - how many terrain profile vertices we need
    //  - how many missing terrain segments there are
    //  - how many flight profile segments we need
    //  - how many terrain collision segments there are
    //  - what is the total distance so we can calculate pixels per meter

    int cTerrainProfilePoints = 0;
    int cMissingTerrainSegments = 0;
    int cFlightProfileSegments = 0;
    int cTerrainCollisionSegments = 0;
    double minTerrainHeight = qQNaN();
    double maxTerrainHeight = qQNaN();

    for (int viIndex = 0; viIndex < _visualItems->count(); viIndex++) {
        const VisualMissionItem *const visualItem = _visualItems->value<VisualMissionItem*>(viIndex);
        const ComplexMissionItem *const complexItem = _visualItems->value<ComplexMissionItem*>(viIndex);

        if (visualItem->simpleFlightPathSegment()) {
            const FlightPathSegment *const segment = visualItem->simpleFlightPathSegment();
            _updateSegmentCounts(segment, cFlightProfileSegments, cTerrainProfilePoints, cMissingTerrainSegments, cTerrainCollisionSegments, minTerrainHeight, maxTerrainHeight);
        }

        if (complexItem) {
            for (int segmentIndex = 0; segmentIndex < complexItem->flightPathSegments()->count(); segmentIndex++) {
                const FlightPathSegment *const segment = complexItem->flightPathSegments()->value<FlightPathSegment*>(segmentIndex);
                _updateSegmentCounts(segment, cFlightProfileSegments, cTerrainProfilePoints, cMissingTerrainSegments, cTerrainCollisionSegments, minTerrainHeight, maxTerrainHeight);
            }
        }
    }

    // The profile view min/max is setup to include a full terrain profile as well as the flight path segments.
    _minAMSLAlt = std::fmin(_missionController->minAMSLAltitude(), minTerrainHeight);
    _maxAMSLAlt = std::fmax(_missionController->maxAMSLAltitude(), maxTerrainHeight);

    // We add a buffer to the min/max alts such that the visuals don't draw lines right at the edges of the display
    double amslAltRange = _maxAMSLAlt - _minAMSLAlt;
    const double amslAltRangeBuffer = amslAltRange * 0.1;
    _maxAMSLAlt += amslAltRangeBuffer;
    if (_minAMSLAlt > 0.0) {
        _minAMSLAlt -= amslAltRangeBuffer;
        _minAMSLAlt = std::fmax(_minAMSLAlt, 0.0);
    }
    amslAltRange = _maxAMSLAlt - _minAMSLAlt;

    static int counter = 0;
    qCDebug(TerrainProfileLog) << "missionController min/max" << _missionController->minAMSLAltitude() << _missionController->maxAMSLAltitude();
    qCDebug(TerrainProfileLog) << QStringLiteral("updatePaintNode counter:%1 cFlightProfileSegments:%2 cTerrainProfilePoints:%3 cMissingTerrainSegments:%4 cTerrainCollisionSegments:%5 _minAMSLAlt:%6 _maxAMSLAlt:%7 maxTerrainHeight:%8")
                                  .arg(counter++).arg(cFlightProfileSegments).arg(cTerrainProfilePoints).arg(cMissingTerrainSegments).arg(cTerrainCollisionSegments).arg(_minAMSLAlt).arg(_maxAMSLAlt).arg(maxTerrainHeight);

    _pixelsPerMeter = _visibleWidth / _missionController->missionTotalDistance();

    // Instantiate nodes
    QSGNode *rootNode = static_cast<QSGNode*>(oldNode);
    QSGGeometry *terrainProfileGeometry = nullptr;
    QSGGeometry *missingTerrainGeometry = nullptr;
    QSGGeometry *flightProfileGeometry = nullptr;
    QSGGeometry *terrainCollisionGeometry = nullptr;
    if (!rootNode) {
        rootNode = new QSGNode();

        QSGGeometryNode *terrainProfileNode = nullptr;
        QSGGeometryNode *missingTerrainNode = nullptr;
        QSGGeometryNode *flightProfileNode = nullptr;
        QSGGeometryNode *terrainCollisionNode = nullptr;

        _createGeometry(terrainProfileNode, terrainProfileGeometry, QSGGeometry::DrawLineStrip, "green");
        _createGeometry(missingTerrainNode, missingTerrainGeometry, QSGGeometry::DrawLines, "yellow");
        _createGeometry(flightProfileNode, flightProfileGeometry, QSGGeometry::DrawLines, "orange");
        _createGeometry(terrainCollisionNode, terrainCollisionGeometry, QSGGeometry::DrawLines, "red");

        rootNode->appendChildNode(terrainProfileNode);
        rootNode->appendChildNode(missingTerrainNode);
        rootNode->appendChildNode(flightProfileNode);
        rootNode->appendChildNode(terrainCollisionNode);
    }

    // Allocate space for the vertices

    QSGNode *node = rootNode->childAtIndex(0);
    terrainProfileGeometry = static_cast<QSGGeometryNode*>(node)->geometry();
    terrainProfileGeometry->allocate(cTerrainProfilePoints);
    node->markDirty(QSGNode::DirtyGeometry);

    node = rootNode->childAtIndex(1);
    missingTerrainGeometry = static_cast<QSGGeometryNode*>(node)->geometry();
    missingTerrainGeometry->allocate(cMissingTerrainSegments * 2);
    node->markDirty(QSGNode::DirtyGeometry);

    node = rootNode->childAtIndex(2);
    flightProfileGeometry = static_cast<QSGGeometryNode*>(node)->geometry();
    flightProfileGeometry->allocate(cFlightProfileSegments * 2);
    node->markDirty(QSGNode::DirtyGeometry);

    node = rootNode->childAtIndex(3);
    terrainCollisionGeometry = static_cast<QSGGeometryNode*>(node)->geometry();
    terrainCollisionGeometry->allocate(cTerrainCollisionSegments * 2);
    node->markDirty(QSGNode::DirtyGeometry);

    int flightProfileVertexIndex = 0;
    int terrainProfileVertexIndex = 0;
    int missingterrainProfileVertexIndex = 0;
    int terrainCollisionVertexIndex = 0;
    double currentDistance = 0;
    QSGGeometry::Point2D *flightProfileVertices = flightProfileGeometry->vertexDataAsPoint2D();
    QSGGeometry::Point2D *terrainProfileVertices = terrainProfileGeometry->vertexDataAsPoint2D();
    QSGGeometry::Point2D *missingTerrainVertices = missingTerrainGeometry->vertexDataAsPoint2D();
    QSGGeometry::Point2D *terrainCollisionVertices = terrainCollisionGeometry->vertexDataAsPoint2D();

    // This step places the vertices for display into the nodes
    for (int viIndex = 0; viIndex < _visualItems->count(); viIndex++) {
        const VisualMissionItem *const visualItem = _visualItems->value<VisualMissionItem*>(viIndex);
        const ComplexMissionItem *const complexItem = _visualItems->value<ComplexMissionItem*>(viIndex);

        if (complexItem) {
            if (complexItem->flightPathSegments()->count() == 0) {
                currentDistance += complexItem->complexDistance();
            } else {
                for (int segmentIndex = 0; segmentIndex < complexItem->flightPathSegments()->count(); segmentIndex++) {
                    const FlightPathSegment *const  segment = complexItem->flightPathSegments()->value<FlightPathSegment*>(segmentIndex);

                    _addFlightProfileSegment(segment, currentDistance, amslAltRange, flightProfileVertices, flightProfileVertexIndex);
                    _addTerrainProfileSegment(segment, currentDistance, amslAltRange, terrainProfileVertices, terrainProfileVertexIndex);
                    _addMissingTerrainSegment(segment, currentDistance, missingTerrainVertices, missingterrainProfileVertexIndex);
                    _addTerrainCollisionSegment(segment, currentDistance, amslAltRange, terrainCollisionVertices, terrainCollisionVertexIndex);

                    currentDistance += segment->totalDistance();
                }
            }
        }

        if (visualItem->simpleFlightPathSegment()) {
            const FlightPathSegment *const segment = visualItem->simpleFlightPathSegment();

            _addFlightProfileSegment(segment, currentDistance, amslAltRange, flightProfileVertices, flightProfileVertexIndex);
            _addTerrainProfileSegment(segment, currentDistance, amslAltRange, terrainProfileVertices, terrainProfileVertexIndex);
            _addMissingTerrainSegment(segment, currentDistance, missingTerrainVertices, missingterrainProfileVertexIndex);
            _addTerrainCollisionSegment(segment, currentDistance, amslAltRange, terrainCollisionVertices, terrainCollisionVertexIndex);

            currentDistance += segment->totalDistance();
        }
    }

    setImplicitWidth(_visibleWidth/*(_totalDistance * pixelsPerMeter) + (_horizontalMargin * 2)*/);
    setWidth(implicitWidth());

    emit implicitWidthChanged();
    emit widthChanged();
    emit pixelsPerMeterChanged();
    emit minAMSLAltChanged();
    emit maxAMSLAltChanged();

    return rootNode;
}

bool TerrainProfile::_shouldAddFlightProfileSegment(const FlightPathSegment *segment)
{
    bool shouldAdd = !qIsNaN(segment->coord1AMSLAlt()) && !qIsNaN(segment->coord2AMSLAlt());
    if (segment->segmentType() == FlightPathSegment::SegmentTypeTerrainFrame) {
        shouldAdd &= !segment->amslTerrainHeights().isEmpty();
    }
    return shouldAdd;
}

bool TerrainProfile::_shouldAddMissingTerrainSegment(const FlightPathSegment *segment)
{
    return segment->amslTerrainHeights().isEmpty();
}
