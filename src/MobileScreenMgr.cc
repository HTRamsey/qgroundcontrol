/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "MobileScreenMgr.h"
#include <QtGlobal>
#ifdef Q_OS_ANDROID
    #include "AndroidInterface.h"
#endif

void MobileScreenMgr::setKeepScreenOn( bool keepScreenOn )
{
    #ifdef Q_OS_ANDROID
        AndroidInterface::setKeepScreenOn( keepScreenOn );
    #endif
}
