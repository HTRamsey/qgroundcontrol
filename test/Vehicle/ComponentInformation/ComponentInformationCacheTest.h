/****************************************************************************
 *
 * (c) 2021 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "UnitTest.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QString>

class ComponentInformationCacheTest : public UnitTest
{
    Q_OBJECT

public:
    ComponentInformationCacheTest();

private slots:
    void _basic_test();
    void _lru_test();
    void _multi_test();

private:
    void _setup();
    void _cleanup();

    struct TmpFile {
        QString path;
        QString cacheTag;
        QString content;
        QString cachedPath;
    };

    QVector<TmpFile> _tmpFiles;

    const QString _cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QLatin1String("/QGCCacheTest");
    const QString _tmpFilesDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QLatin1String("/QGCTestFiles");
};

