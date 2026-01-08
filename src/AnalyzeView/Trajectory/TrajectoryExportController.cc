#include "TrajectoryExportController.h"
#include "CsvTrajectoryParser.h"
#include "MovingFeaturesDocument.h"
#include "QGCLoggingCategory.h"
#include "TrajectoryPoints.h"
#include "ULogTrajectoryParser.h"
#include "Vehicle.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

QGC_LOGGING_CATEGORY(TrajectoryExportControllerLog, "AnalyzeView.TrajectoryExportController")

TrajectoryExportController::TrajectoryExportController(QObject* parent)
    : QObject(parent)
{
}

void TrajectoryExportController::setSourceFile(const QString& file)
{
    if (_sourceFile != file) {
        _sourceFile = file;
        emit sourceFileChanged();
        setErrorMessage({});
        setPointCount(0);
    }
}

void TrajectoryExportController::setOutputFile(const QString& file)
{
    if (_outputFile != file) {
        _outputFile = file;
        emit outputFileChanged();
    }
}

void TrajectoryExportController::setSourceType(SourceType type)
{
    if (_sourceType != type) {
        _sourceType = type;
        emit sourceTypeChanged();
        setErrorMessage({});
        setPointCount(0);
    }
}

void TrajectoryExportController::setErrorMessage(const QString& msg)
{
    if (_errorMessage != msg) {
        _errorMessage = msg;
        emit errorMessageChanged();
    }
}

void TrajectoryExportController::setProgress(double progress)
{
    if (!qFuzzyCompare(_progress, progress)) {
        _progress = progress;
        emit progressChanged();
    }
}

void TrajectoryExportController::setInProgress(bool inProgress)
{
    if (_inProgress != inProgress) {
        _inProgress = inProgress;
        emit inProgressChanged();
    }
}

void TrajectoryExportController::setPointCount(int count)
{
    if (_pointCount != count) {
        _pointCount = count;
        emit pointCountChanged();
    }
}

void TrajectoryExportController::exportLiveTrajectory(Vehicle* vehicle)
{
    if (!vehicle) {
        setErrorMessage(tr("No active vehicle"));
        return;
    }

    if (_outputFile.isEmpty()) {
        setErrorMessage(tr("No output file specified"));
        return;
    }

    setInProgress(true);
    setErrorMessage({});
    setProgress(0);

    TrajectoryPoints* trajectoryPoints = vehicle->trajectoryPoints();
    if (!trajectoryPoints) {
        setErrorMessage(tr("No trajectory data available"));
        setInProgress(false);
        return;
    }

    QList<TimestampedTrajectoryPoint> points = trajectoryPoints->timestampedPoints();
    setPointCount(points.count());
    setProgress(25);

    if (points.count() < 2) {
        setErrorMessage(tr("Trajectory requires at least 2 points (have %1)").arg(points.count()));
        setInProgress(false);
        return;
    }

    MovingFeaturesDocument doc;
    doc.setVehicleId(vehicle->id());
    doc.addPoints(points);
    doc.setStartTime(points.first().timestamp);
    doc.setEndTime(points.last().timestamp);
    setProgress(50);

    QString validationError;
    if (!doc.isValid(&validationError)) {
        setErrorMessage(validationError);
        setInProgress(false);
        return;
    }

    QFile outFile(_outputFile);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setErrorMessage(tr("Cannot write to file: %1").arg(_outputFile));
        setInProgress(false);
        return;
    }

    outFile.write(doc.toJson());
    outFile.close();
    setProgress(100);

    qCDebug(TrajectoryExportControllerLog) << "Exported" << points.count()
                                           << "points to" << _outputFile;

    setInProgress(false);
    emit exportComplete(_outputFile);
}

void TrajectoryExportController::exportToFile()
{
    if (_sourceFile.isEmpty()) {
        setErrorMessage(tr("No source file specified"));
        return;
    }

    if (_outputFile.isEmpty()) {
        setErrorMessage(tr("No output file specified"));
        return;
    }

    setInProgress(true);
    setErrorMessage({});
    setProgress(0);

    QList<TimestampedTrajectoryPoint> points;
    QDateTime startTime;
    QDateTime endTime;
    int vehicleId = 0;
    QString parseError;

    if (_sourceType == CsvFile) {
        CsvTrajectoryParser parser;
        setProgress(10);

        if (!parser.parseFile(_sourceFile, parseError)) {
            setErrorMessage(parseError);
            setInProgress(false);
            return;
        }

        points = parser.trajectoryPoints();
        startTime = parser.startTime();
        endTime = parser.endTime();
        vehicleId = parser.vehicleId();
    } else if (_sourceType == ULogFile) {
        ULogTrajectoryParser parser;
        setProgress(10);

        if (!parser.parseFile(_sourceFile, parseError)) {
            setErrorMessage(parseError);
            setInProgress(false);
            return;
        }

        points = parser.trajectoryPoints();
        startTime = parser.startTime();
        endTime = parser.endTime();
    } else {
        setErrorMessage(tr("Invalid source type for file export"));
        setInProgress(false);
        return;
    }

    setPointCount(points.count());
    setProgress(50);

    if (points.count() < 2) {
        setErrorMessage(tr("Trajectory requires at least 2 points (have %1)").arg(points.count()));
        setInProgress(false);
        return;
    }

    MovingFeaturesDocument doc;
    doc.setVehicleId(vehicleId);
    doc.addPoints(points);
    doc.setStartTime(startTime);
    doc.setEndTime(endTime);

    // Add source file info
    QFileInfo fi(_sourceFile);
    doc.setProperty(QStringLiteral("sourceFile"), fi.fileName());
    setProgress(75);

    QString validationError;
    if (!doc.isValid(&validationError)) {
        setErrorMessage(validationError);
        setInProgress(false);
        return;
    }

    QFile outFile(_outputFile);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setErrorMessage(tr("Cannot write to file: %1").arg(_outputFile));
        setInProgress(false);
        return;
    }

    outFile.write(doc.toJson());
    outFile.close();
    setProgress(100);

    qCDebug(TrajectoryExportControllerLog) << "Exported" << points.count()
                                           << "points from" << _sourceFile
                                           << "to" << _outputFile;

    setInProgress(false);
    emit exportComplete(_outputFile);
}

void TrajectoryExportController::cancel()
{
    // Currently export is synchronous, so this is a no-op
    // Would be used if we implement threaded export
    setInProgress(false);
}

QString TrajectoryExportController::preview()
{
    if (_sourceFile.isEmpty() && _sourceType != LiveTrajectory) {
        setErrorMessage(tr("No source file specified"));
        return {};
    }

    setErrorMessage({});

    QList<TimestampedTrajectoryPoint> points;
    QString parseError;

    if (_sourceType == CsvFile) {
        CsvTrajectoryParser parser;
        if (!parser.parseFile(_sourceFile, parseError)) {
            setErrorMessage(parseError);
            return {};
        }
        points = parser.trajectoryPoints();
    } else if (_sourceType == ULogFile) {
        ULogTrajectoryParser parser;
        if (!parser.parseFile(_sourceFile, parseError)) {
            setErrorMessage(parseError);
            return {};
        }
        points = parser.trajectoryPoints();
    }

    setPointCount(points.count());

    if (points.count() < 2) {
        setErrorMessage(tr("Trajectory requires at least 2 points (have %1)").arg(points.count()));
        return {};
    }

    MovingFeaturesDocument doc;
    doc.addPoints(points);

    return QString::fromUtf8(doc.toJson());
}
