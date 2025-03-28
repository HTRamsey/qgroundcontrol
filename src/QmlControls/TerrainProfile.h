/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QLoggingCategory>
#include <QtQuick/QQuickItem>
#include <QtQuick/QSGGeometry>

Q_DECLARE_LOGGING_CATEGORY(TerrainProfileLog)

class MissionController;
class QmlObjectListModel;
class FlightPathSegment;
class QSGGeometryNode;

Q_MOC_INCLUDE("MissionController.h")

class TerrainProfile : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(double               visibleWidth        MEMBER _visibleWidth                                NOTIFY visibleWidthChanged)
    Q_PROPERTY(MissionController    *missionController  READ   missionController WRITE setMissionController NOTIFY missionControllerChanged)
    Q_PROPERTY(double               pixelsPerMeter      MEMBER _pixelsPerMeter                              NOTIFY pixelsPerMeterChanged)
    Q_PROPERTY(double               minAMSLAlt          MEMBER _minAMSLAlt                                  NOTIFY minAMSLAltChanged)
    Q_PROPERTY(double               maxAMSLAlt          MEMBER _maxAMSLAlt                                  NOTIFY maxAMSLAltChanged)

public:
    TerrainProfile(QQuickItem *parent = nullptr);

    MissionController *const missionController() { return _missionController; }
    void setMissionController(MissionController *missionController);

    QSGNode *updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *updatePaintNodeData) final;

    void componentComplete() final;

signals:
    void missionControllerChanged();
    void visibleWidthChanged();
    void pixelsPerMeterChanged();
    void minAMSLAltChanged();
    void maxAMSLAltChanged();
    void updateSignal();

private slots:
    void _newVisualItems();

private:
    static void _createGeometry(QSGGeometryNode *&geometryNode, QSGGeometry *&geometry, QSGGeometry::DrawingMode drawingMode, const QColor &color);
    static void _updateSegmentCounts(const FlightPathSegment *segment, int &cFlightProfileSegments, int &cTerrainPoints, int &cMissingTerrainSegments, int &cTerrainCollisionSegments, double &minTerrainHeight, double &maxTerrainHeight);
    void _addTerrainProfileSegment(const FlightPathSegment *segment, double currentDistance, double amslAltRange, QSGGeometry::Point2D *terrainProfileVertices, int &terrainVertexIndex) const;
    void _addMissingTerrainSegment(const FlightPathSegment *segment, double currentDistance, QSGGeometry::Point2D *missingTerrainVertices, int &missingTerrainVertexIndex) const;
    void _addTerrainCollisionSegment(const FlightPathSegment *segment, double currentDistance, double amslAltRange, QSGGeometry::Point2D *terrainCollisionVertices, int &terrainCollisionVertexIndex) const;
    void _addFlightProfileSegment(const FlightPathSegment *segment, double currentDistance, double amslAltRange, QSGGeometry::Point2D *flightProfileVertices, int &flightProfileVertexIndex) const;
    static bool _shouldAddFlightProfileSegment(const FlightPathSegment *segment);
    static bool _shouldAddMissingTerrainSegment(const FlightPathSegment *segment);

    MissionController *_missionController = nullptr;
    QmlObjectListModel *_visualItems = nullptr;
    double _visibleWidth = 0;
    double _pixelsPerMeter = 0;
    double _minAMSLAlt = 0;
    double _maxAMSLAlt = 0;

    static constexpr int _lineWidth = 7;

    Q_DISABLE_COPY(TerrainProfile)
};
QML_DECLARE_TYPE(TerrainProfile)
