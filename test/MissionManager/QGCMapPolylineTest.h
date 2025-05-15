/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "UnitTest.h"

class QmlObjectListModel;
class QGCMapPolyline;
class MultiSignalSpyV2;

class QGCMapPolylineTest : public UnitTest
{
    Q_OBJECT

protected:
    void init() final;

private slots:
    void _testDirty();
    void _testVertexManipulation();
//    void _testKMLLoad(void);
    void _testSelectVertex();

private:
    enum {
        countChangedIndex = 0,
        pathChangedIndex,
        dirtyChangedIndex,
        clearedIndex,
        maxSignalIndex
    };

    enum {
        countChangedMask = 1 << countChangedIndex,
        pathChangedMask = 1 << pathChangedIndex,
        dirtyChangedMask = 1 << dirtyChangedIndex,
        clearedMask = 1 << clearedIndex,
    };

    enum {
        modelCountChangedIndex = 0,
        modelDirtyChangedIndex,
        maxModelSignalIndex
    };

    enum {
        modelCountChangedMask = 1 << modelCountChangedIndex,
        modelDirtyChangedMask = 1 << modelDirtyChangedIndex,
    };

    MultiSignalSpyV2 *_multiSpyPolyline = nullptr;
    MultiSignalSpyV2 *_multiSpyModel = nullptr;
    QGCMapPolyline *_mapPolyline = nullptr;
    QmlObjectListModel *_pathModel = nullptr;
    const QList<QGeoCoordinate> _linePoints = {
        QGeoCoordinate(47.635638361473475, -122.09269407980834),
        QGeoCoordinate(47.635638361473475, -122.08545246602667),
        QGeoCoordinate(47.63057923872075, -122.08545246602667),
        QGeoCoordinate(47.63057923872075, -122.09269407980834)
    };
};
