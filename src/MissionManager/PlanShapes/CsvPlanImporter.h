#pragma once

#include "PlanImporter.h"

/// Imports waypoints from CSV files
/// Expects columns: Name, Latitude, Longitude, Altitude (or any subset)
/// First row is treated as header if it contains text in latitude column
class CsvPlanImporter : public PlanImporter
{
    Q_OBJECT

public:
    PlanImportResult importFromFile(const QString& filename) override;

    QString fileExtension() const override { return QStringLiteral("csv"); }
    QString formatName() const override { return tr("Comma-Separated Values"); }
    QString fileFilter() const override { return tr("CSV Files (*.csv)"); }

private:
    explicit CsvPlanImporter(QObject* parent = nullptr);

    bool _parseLine(const QString& line, QChar delimiter, QStringList& fields);
    int _findColumn(const QStringList& headers, const QStringList& possibleNames);

    DECLARE_PLAN_IMPORTER_SINGLETON(CsvPlanImporter)
};
