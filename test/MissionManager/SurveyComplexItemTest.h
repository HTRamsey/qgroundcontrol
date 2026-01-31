#pragma once

#include "TransectStyleComplexItemTestBase.h"

#include <QtPositioning/QGeoCoordinate>

class SurveyComplexItem;
class QGCMapPolygon;
class MultiSignalSpy;

/// Unit test for SurveyComplexItem
class SurveyComplexItemTest : public TransectStyleComplexItemTestBase
{
    Q_OBJECT

public:
    SurveyComplexItemTest(void);

protected:
    void init(void) final;
    void cleanup(void) final;

#if 1
private slots:
    void _testDirty(void);
    void _testGridAngle(void);
    void _testEntryLocation(void);
    void _testItemGeneration(void);
    void _testItemCount(void);
    void _testHoverCaptureItemGeneration(void);
#else
    // Handy mechanism to to a single test
private slots:
    void _testItemCount(void);
private:
    void _testDirty(void);
    void _testGridAngle(void);
    void _testEntryLocation(void);
    void _testItemGeneration(void);
    void _testHoverCaptureItemGeneration(void);
#endif

private:
    double          _clampGridAngle180(double gridAngle);
    QList<MAV_CMD>  _createExpectedCommands(bool hasTurnaround, bool useConditionGate);
    void            _testItemGenerationWorker(bool imagesInTurnaround, bool hasTurnaround, bool useConditionGate, const QList<MAV_CMD>& expectedCommands);

    MultiSignalSpy*         _multiSpy =             nullptr;
    SurveyComplexItem*      _surveyItem =           nullptr;
    QGCMapPolygon*          _mapPolygon =           nullptr;
    QList<QGeoCoordinate>   _polyVertices;

    static const int _expectedTransectCount = 2;
};
