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

#include <QtPositioning/QGeoCoordinate>

class GeoTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _convertGeoToNed_test();
    void _convertGeoToNedAtOrigin_test();
    void _convertNedToGeo_test();
    void _convertNedToGeoAtOrigin_test();

    void _convertGeoToUTM_test();
    void _convertUTMToGeo_test();
    void _convertGeoToMGRS_test();
    void _convertMGRSToGeo_test();

private:
    /// Use ETH campus (47.3764° N, 8.5481° E)
    const QGeoCoordinate m_origin{47.3764, 8.5481, 0.0};
};
