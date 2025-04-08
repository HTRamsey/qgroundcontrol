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

class MavlinkLogTest : public UnitTest
{
    Q_OBJECT

signals:
    void checkForLostLogFiles();

private slots:
    void init();
    void cleanup();

    void _bootLogDetectionCancel_test();
    void _bootLogDetectionSave_test();
    void _bootLogDetectionZeroLength_test();
    void _connectLogNoArm_test();
    void _connectLogArm_test();
    void _deleteTempLogFiles_test();

private:
    void _createTempLogFile(bool zeroLength);
    void _connectLogWorker(bool arm);

    static constexpr const char *_tempLogFileTemplate = "FlightDataXXXXXX";         ///< Template for temporary log file
    static constexpr const char *_logFileExtension = "mavlink";                     ///< Extension for log files
    static constexpr const char *_saveLogFilename = "qgroundcontrol.mavlink.ut";    ///< Filename to save log files to
};

