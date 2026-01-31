#pragma once

#include "UnitTest.h"

class QmlObjectListModel;
class QGCMapPolygon;
class MultiSignalSpy;

class QGCMapPolygonTest : public UnitTest
{
    Q_OBJECT

public:
    QGCMapPolygonTest(void);

protected:
    void init(void) final;
    void cleanup(void) final;

private slots:
    void _testDirty(void);
    void _testVertexManipulation(void);
    void _testKMLLoad(void);
    void _testSelectVertex(void);
    void _testSegmentSplit(void);

private:
    MultiSignalSpy*         _multiSpyPolygon = nullptr;
    MultiSignalSpy*         _multiSpyModel = nullptr;
    QGCMapPolygon*          _mapPolygon = nullptr;
    QmlObjectListModel*     _pathModel = nullptr;
    QList<QGeoCoordinate>   _polyPoints;
};
