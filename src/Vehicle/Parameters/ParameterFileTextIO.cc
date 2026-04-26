#include "ParameterFileTextIO.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include <algorithm>

#include "Fact.h"
#include "FactMetaData.h"
#include "ParameterManager.h"
#include "ParameterMavlinkCodec.h"
#include "ParameterTextLine.h"
#include "QGCLoggingCategory.h"
#include "Vehicle.h"

QGC_LOGGING_CATEGORY(ParameterFileTextIOLog, "Vehicle.Parameters.ParameterFileTextIO")

namespace ParameterFileTextIO {

namespace {
constexpr int kMaxErrorListItems = 20;

QString joinCapped(const QStringList& items)
{
    if (items.size() <= kMaxErrorListItems) {
        return items.join(QStringLiteral(", "));
    }
    const QStringList head = items.mid(0, kMaxErrorListItems);
    return QCoreApplication::translate("ParameterFileTextIO", "%1, …and %2 more")
        .arg(head.join(QStringLiteral(", ")))
        .arg(items.size() - kMaxErrorListItems);
}
}  // namespace

QString read(QTextStream& stream, ParameterManager* manager)
{
    QList<ParameterTextLine::ParsedRow> rows;
    int malformedCount = 0;
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }
        const auto parsed = ParameterTextLine::parseLine(line);
        if (!parsed) {
            ++malformedCount;
            qCDebug(ParameterFileTextIOLog) << "Malformed line (skipped):" << line;
            continue;
        }
        rows.push_back(*parsed);
    }

    // Validate vehicle id up-front before mutating any Facts so a mismatched row N
    // doesn't leave rows 0..N-1 already applied.
    if (!rows.isEmpty() && manager->vehicle()->id() != rows.first().vehicleId) {
        return QCoreApplication::translate("ParameterFileTextIO",
                                           "The parameters in the stream have been saved from System Id %1, but "
                                           "the current vehicle has the System Id %2.")
            .arg(rows.first().vehicleId)
            .arg(manager->vehicle()->id());
    }

    QStringList missingErrors;
    QStringList typeErrors;
    QStringList outOfRangeWarnings;
    for (const auto& parsed : rows) {
        if (!manager->parameterExists(parsed.componentId, parsed.name)) {
            const QString error = QStringLiteral("%1:%2").arg(parsed.componentId).arg(parsed.name);
            missingErrors << error;
            qCDebug(ParameterFileTextIOLog) << "Skipped due to missing:" << error;
            continue;
        }

        Fact* const fact = manager->getParameter(parsed.componentId, parsed.name);
        if (fact->type() != ParameterMavlinkCodec::mavTypeToFactType(parsed.type)) {
            const QString error = QStringLiteral("%1:%2").arg(parsed.componentId).arg(parsed.name);
            typeErrors << error;
            qCDebug(ParameterFileTextIOLog) << "Skipped due to type mismatch:" << error;
            continue;
        }

        // Best-effort raw-bounds sanity check: if metadata reports min/max and the loaded value
        // sits outside, surface a warning so users can spot edited-by-hand files. We still apply
        // the value — text-load is intentionally raw-fidelity — but the result string lists it.
        if (FactMetaData* const md = fact->metaData()) {
            QVariant typedValue;
            QString boundsErrorString;
            if (!md->convertAndValidateRaw(QVariant(parsed.rawValue), /*convertOnly*/ false,
                                           typedValue, boundsErrorString)) {
                const QString warning = QStringLiteral("%1:%2 (%3)")
                                            .arg(parsed.componentId)
                                            .arg(parsed.name, boundsErrorString);
                outOfRangeWarnings << warning;
                qCDebug(ParameterFileTextIOLog) << "Out-of-range:" << warning;
            }
        }

        qCDebug(ParameterFileTextIOLog) << "Updating parameter" << parsed.componentId << parsed.name
                                        << parsed.rawValue;
        fact->setRawValue(parsed.rawValue);
    }

    QString errors;
    if (!missingErrors.isEmpty()) {
        errors = QCoreApplication::translate("ParameterFileTextIO",
                                             "Parameters not loaded since they are not currently on the vehicle: %1\n")
                     .arg(joinCapped(missingErrors));
    }
    if (!typeErrors.isEmpty()) {
        errors += QCoreApplication::translate("ParameterFileTextIO", "Parameters not loaded due to type mismatch: %1\n")
                      .arg(joinCapped(typeErrors));
    }
    if (malformedCount > 0) {
        errors += QCoreApplication::translate("ParameterFileTextIO", "Skipped %1 malformed line(s).\n")
                      .arg(malformedCount);
    }
    if (!outOfRangeWarnings.isEmpty()) {
        errors += QCoreApplication::translate("ParameterFileTextIO",
                                              "Loaded values outside metadata range: %1\n")
                      .arg(joinCapped(outOfRangeWarnings));
    }
    return errors;
}

void write(QTextStream& stream, ParameterManager* manager)
{
    Vehicle* const vehicle = manager->vehicle();

    stream << "# Onboard parameters for Vehicle " << vehicle->id() << "\n";
    stream << "#\n";
    stream << "# Stack: " << vehicle->firmwareTypeString() << "\n";
    stream << "# Vehicle: " << vehicle->vehicleTypeString() << "\n";
    stream << "# Version: " << vehicle->firmwareMajorVersion() << "." << vehicle->firmwareMinorVersion() << "."
           << vehicle->firmwarePatchVersion() << " " << vehicle->firmwareVersionTypeString() << "\n";
    stream << "# Git Revision: " << vehicle->gitHash() << "\n";
    stream << "#\n";
    stream << "# Vehicle-Id Component-Id Name Value Type\n";

    // Sort ids and names so two consecutive saves diff cleanly — QHash iteration is randomized.
    QList<int> compIds = manager->componentIds();
    std::sort(compIds.begin(), compIds.end());
    for (const int componentId : compIds) {
        QStringList names = manager->parameterNames(componentId);
        std::sort(names.begin(), names.end());
        for (const QString& paramName : names) {
            Fact* const fact = manager->getParameter(componentId, paramName);
            if (!fact) {
                qCWarning(ParameterFileTextIOLog) << "Internal error: missing fact" << componentId << paramName;
                continue;
            }
            // rawValueStringFullPrecision() routes through QString::arg(double, …) which is
            // C-locale by contract, so saved files are portable across user locales.
            stream << ParameterTextLine::formatRow(
                vehicle->id(), componentId, paramName,
                fact->rawValueStringFullPrecision(),
                ParameterMavlinkCodec::factTypeToMavType(fact->type()));
        }
    }

    stream.flush();
}

}  // namespace ParameterFileTextIO
