#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QStringList>

#include "QGCCommandLineParser.h"

Q_DECLARE_LOGGING_CATEGORY(UnitTestListLog)

namespace QGCUnitTest {

/// Number of iterations for stress testing
constexpr int kStressIterations = 20;

/// Runs registered unit tests
/// @param stress If true, runs tests kStressIterations times to catch intermittent failures
/// @param unitTests List of test names to run (must not be empty)
/// @param outputFile Optional output file for JUnit XML results (empty = console only)
/// @return 0 on success, negative number indicating failure count
int runTests(bool stress, const QStringList& unitTests, const QString& outputFile = QString());

/// Returns list of all registered test names
QStringList registeredTestNames();

/// Returns total number of registered tests
int registeredTestCount();

/// Checks if a test with the given name is registered
/// @param testName Test name to check (case-sensitive)
/// @return true if test is registered
bool isTestRegistered(const QString& testName);

/// Validates that all test names in the list are registered
/// @param testNames List of test names to validate
/// @return List of invalid test names (empty if all valid)
QStringList validateTestNames(const QStringList& testNames);

/// Handle all test-related command line options (--list-tests, --unittest, etc.)
/// @param args Parsed command line arguments
/// @return 0 on success, negative number indicating failure count
int handleTestOptions(const QGCCommandLineParser::CommandLineParseResult& args);

}
