/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "FactSystemTestGeneric.h"
#include "QGCMAVLink.h"

void FactSystemTestGeneric::init()
{
    UnitTest::init();
    _init(MAV_AUTOPILOT_GENERIC);
}
