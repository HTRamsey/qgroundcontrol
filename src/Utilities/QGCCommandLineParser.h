#pragma once

#include <QtCore/QCommandLineParser>

namespace QGCCommandLineParser
{

struct CommandLineParseResult
{
    QCommandLineParser parser;

    enum class Status {
        Ok,
        Error,
        VersionRequested,
        HelpRequested
    };
    Status statusCode = Status::Ok;
    std::optional<QString> errorString = std::nullopt;

    bool runningUnitTests = false;
    QStringList unitTests;
    bool stressUnitTests = false;
    unsigned stressUnitTestsCount = 0;
    bool clearSettingsOptions = false;
    bool clearCache = false;
    bool logging = false;
    QString loggingOptions;
    bool fakeMobile = false;
    bool logOutput = false;
#ifdef Q_OS_WIN
    bool quietWindowsAsserts = false;
#endif
};

void parseCommandLine(CommandLineParseResult &parseResult);

}; // QGCCommandLineParser
