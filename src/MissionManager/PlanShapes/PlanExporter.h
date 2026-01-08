#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(PlanExporterLog)

class MissionController;

/// Abstract base class for plan exporters
class PlanExporter : public QObject
{
    Q_OBJECT

public:
    explicit PlanExporter(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~PlanExporter() = default;

    /// Export the plan to the specified file
    /// @param filename Output file path
    /// @param missionController Mission controller containing the plan data
    /// @param errorString Output parameter for error message on failure
    /// @return true on success
    virtual bool exportToFile(const QString& filename,
                              MissionController* missionController,
                              QString& errorString) = 0;

    /// Returns the file extension for this exporter (without dot)
    virtual QString fileExtension() const = 0;

    /// Returns a human-readable name for this format
    virtual QString formatName() const = 0;

    /// Returns the file filter string for file dialogs
    virtual QString fileFilter() const = 0;

    // ========================================================================
    // Static factory methods
    // ========================================================================

    /// Register an exporter for a file extension
    static void registerExporter(const QString& extension, PlanExporter* exporter);

    /// Get an exporter for the given file extension (returns nullptr if none registered)
    static PlanExporter* exporterForExtension(const QString& extension);

    /// Get an exporter for the given filename (based on extension)
    static PlanExporter* exporterForFile(const QString& filename);

    /// Returns list of all registered file extensions
    static QStringList registeredExtensions();

    /// Returns combined file filter string for all exporters
    static QStringList fileDialogFilters();

    /// Initialize all built-in exporters
    static void initializeExporters();

private:
    static QHash<QString, PlanExporter*> s_exporters;
    static bool s_initialized;
};
