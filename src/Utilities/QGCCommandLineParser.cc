#include "QGCCommandLineParser.h"

#include <QtCore/QCoreApplication>

namespace QGCCommandLineParser
{

void parseCommandLine(CommandLineParseResult &parseResult)
{
    QCommandLineParser* const parser = &parseResult.parser;

    parser->setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser->setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);

    parser->setApplicationDescription("Open Source Ground Control App");
    const QCommandLineOption helpOption = parser->addHelpOption();
    const QCommandLineOption versionOption = parser->addVersionOption();

    const QCommandLineOption runUnitTestsOption("unittest", "Run unit tests.", "unittests");
    parser->addOption(runUnitTestsOption);

    const QCommandLineOption stressUnitTestsOption("unittest-stress", "Stress test unit tests.", "count");
    parser->addOption(stressUnitTestsOption);

#ifdef Q_OS_WIN
    const QCommandLineOption quietWindowsAssertsOption("no-windows-assert-ui", "Don't let asserts pop dialog boxes.");
    parser->addOption(quietWindowsAssertsOption);
#endif

    const QCommandLineOption clearSettingsOption("clear-settings", "Clear stored settings.");
    parser->addOption(clearSettingsOption);

    const QCommandLineOption clearCacheOption("clear-cache", "Clear parameter/airframe caches.");
    parser->addOption(clearCacheOption);

    const QCommandLineOption loggingOption("logging", "Turn on logging.", "options");
    parser->addOption(loggingOption);

    const QCommandLineOption fakeMobileOption("fake-mobile", "Fake Mobile.");
    parser->addOption(fakeMobileOption);

    const QCommandLineOption logOutputOption("log-output", "Log Output.");
    parser->addOption(logOutputOption);

    parser->process(*QCoreApplication::instance());

    const QStringList unknownOptions = parser->unknownOptionNames();
    if (!unknownOptions.isEmpty()) {
        parseResult.statusCode = CommandLineParseResult::Status::Error;
        parseResult.errorString = "Unknown Options: " + unknownOptions.join(", ");
        return;
    }

    const QStringList args = parser->positionalArguments();
    if (!args.isEmpty()) {
        parseResult.statusCode = CommandLineParseResult::Status::Error;
        parseResult.errorString = "Unexpected Positional Arguments: " + args.join(", ");
        return;
    }

    // if (!parser->parse(QCoreApplication::arguments())) {
    //     result.statusCode = CommandLineParseResult::Error;
    //     result.errorString = parser->errorText();
    //     return;
    // }

    if (parser->isSet(helpOption)) {
        parseResult.statusCode = CommandLineParseResult::Status::HelpRequested;
        return;
    }

    if (parser->isSet(versionOption)) {
        parseResult.statusCode = CommandLineParseResult::Status::VersionRequested;
        return;
    }

    if (parser->isSet(runUnitTestsOption)) {
        parseResult.runningUnitTests = true;
        parseResult.unitTests = parser->values(runUnitTestsOption);
        // Unit tests run with clean settings
        parseResult.clearSettingsOptions = true;
    }

    if (parser->isSet(stressUnitTestsOption)) {
        parseResult.runningUnitTests = parseResult.stressUnitTests = true;
        bool ok;
        parseResult.stressUnitTestsCount = parser->value(stressUnitTestsOption).toUInt(&ok);
        if (!ok) {
            parseResult.statusCode = CommandLineParseResult::Status::Error;
            parseResult.errorString = "Invalid stress unit test count";
            return;
        }
    }

    if (parser->isSet(clearSettingsOption)) {
        parseResult.clearSettingsOptions = true;
    }

    if (parser->isSet(clearCacheOption)) {
        parseResult.clearCache = true;
    }

    if (parser->isSet(loggingOption)) {
        parseResult.logging = true;
        parseResult.loggingOptions = parser->value(loggingOption);
    }

    if (parser->isSet(fakeMobileOption)) {
        parseResult.fakeMobile = true;
    }

    if (parser->isSet(logOutputOption)) {
        parseResult.logOutput = true;
    }

#ifdef Q_OS_WIN
    if (parser->isSet(quietWindowsAssertsOption)) {
        parseResult.quietWindowsAsserts = true;
    }
#endif

    parseResult.statusCode = CommandLineParseResult::Status::Ok;
}

/*bool QGCApplication::_parseCommandLine()
{
    const CommandLineParseResult parseResult = QGCCommandLineParser::parseCommandLine();

    switch (parseResult.statusCode) {
    case Status::Ok:
        _runningUnitTests = parseResult.runningUnitTests;
        _fakeMobile = parseResult.fakeMobile;
        _logOutput = parseResult.logOutput;
        break;
    case Status::Error:
        std::fputs(qPrintable(parseResult.errorString.value_or(u"Unknown error occurred"_s)),
                   stderr);
        std::fputs("\n\n", stderr);
        std::fputs(qPrintable(parseResult.parser.helpText()), stderr);

        QString errorMessage = parseResult.errorString.value_or(u"Unknown error occurred"_qs);
        QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
                             "<html><head/><body><h2>" + errorMessage + "</h2><pre>"
                             + parseResult.parser.helpText() + "</pre></body></html>");
        return false;
    case Status::VersionRequested:
        parseResult.parser.showVersion();
        QMessageBox::information(0, QGuiApplication::applicationDisplayName(),
                         QGuiApplication::applicationDisplayName() + ' '
                         + QCoreApplication::applicationVersion());
        break;
    case Status::HelpRequested:
        parseResult.parser.showHelp();
        QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
                     "<html><head/><body><pre>"
                     + parseResult.parser.helpText() + "</pre></body></html>");
        break;
    }

    return true;
}*/

} // namespace QGCCommandLineParser
