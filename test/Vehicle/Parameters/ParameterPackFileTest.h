#pragma once

#include <QtCore/QString>
#include <QtCore/QTemporaryDir>

#include "UnitTest.h"

class ParameterPackFileTest : public UnitTest
{
    Q_OBJECT

private slots:
    void init() override;
    void cleanup() override;

    void _parseMissingFile();
    void _parseInvalidMagic();
    void _parsePartialHeaderRejected();
    void _parseSingleInt32();
    void _parseMixedTypes();
    void _parseWithDefaults();
    void _parseSharedNamePrefix();
    void _parseTruncatedValueRejected();
    void _parseNameLengthOverflowRejected();

private:
    /// Write @p bytes into a fresh temp file under _tempDir and return its path.
    QString _writeFixture(const QByteArray& bytes) const;

    QTemporaryDir _tempDir;
    int _fixtureCounter = 0;
};
