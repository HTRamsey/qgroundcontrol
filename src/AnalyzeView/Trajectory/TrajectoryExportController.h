#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtQmlIntegration/QtQmlIntegration>

class Vehicle;

Q_DECLARE_LOGGING_CATEGORY(TrajectoryExportControllerLog)

/// Controller for TrajectoryExportPage.qml
/// Exports flight trajectory to OGC Moving Features JSON format
class TrajectoryExportController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString sourceFile READ sourceFile WRITE setSourceFile NOTIFY sourceFileChanged)
    Q_PROPERTY(QString outputFile READ outputFile WRITE setOutputFile NOTIFY outputFileChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool inProgress READ inProgress NOTIFY inProgressChanged)
    Q_PROPERTY(int pointCount READ pointCount NOTIFY pointCountChanged)
    Q_PROPERTY(SourceType sourceType READ sourceType WRITE setSourceType NOTIFY sourceTypeChanged)

public:
    enum SourceType {
        LiveTrajectory = 0,
        CsvFile = 1,
        ULogFile = 2
    };
    Q_ENUM(SourceType)

    explicit TrajectoryExportController(QObject* parent = nullptr);
    ~TrajectoryExportController() override = default;

    Q_INVOKABLE void exportToFile();
    Q_INVOKABLE void exportLiveTrajectory(Vehicle* vehicle);
    Q_INVOKABLE void cancel();
    Q_INVOKABLE QString preview();

    QString sourceFile() const { return _sourceFile; }
    QString outputFile() const { return _outputFile; }
    QString errorMessage() const { return _errorMessage; }
    double progress() const { return _progress; }
    bool inProgress() const { return _inProgress; }
    int pointCount() const { return _pointCount; }
    SourceType sourceType() const { return _sourceType; }

    void setSourceFile(const QString& file);
    void setOutputFile(const QString& file);
    void setSourceType(SourceType type);

signals:
    void sourceFileChanged();
    void outputFileChanged();
    void errorMessageChanged();
    void progressChanged();
    void inProgressChanged();
    void pointCountChanged();
    void sourceTypeChanged();
    void exportComplete(const QString& filename);

private:
    void setErrorMessage(const QString& msg);
    void setProgress(double progress);
    void setInProgress(bool inProgress);
    void setPointCount(int count);

    QString _sourceFile;
    QString _outputFile;
    QString _errorMessage;
    double _progress = 0.0;
    bool _inProgress = false;
    int _pointCount = 0;
    SourceType _sourceType = CsvFile;
};
