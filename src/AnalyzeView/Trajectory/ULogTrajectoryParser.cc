#include "ULogTrajectoryParser.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFile>

#include <ulog_cpp/data_container.hpp>
#include <ulog_cpp/reader.hpp>

using namespace ulog_cpp;

QGC_LOGGING_CATEGORY(ULogTrajectoryParserLog, "AnalyzeView.ULogTrajectoryParser")

ULogTrajectoryParser::ULogTrajectoryParser() = default;

bool ULogTrajectoryParser::parseFile(const QString& filename, QString& errorMessage)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = QObject::tr("Cannot open file: %1").arg(filename);
        return false;
    }

    QByteArray logData = file.readAll();
    return parse(logData, errorMessage);
}

bool ULogTrajectoryParser::parse(const QByteArray& logData, QString& errorMessage)
{
    errorMessage.clear();
    _points.clear();

    std::shared_ptr<DataContainer> data = std::make_shared<DataContainer>(DataContainer::StorageConfig::FullLog);
    Reader parser(data);
    parser.readChunk(reinterpret_cast<const uint8_t*>(logData.constData()), logData.size());

    if (!data->parsingErrors().empty()) {
        for (const std::string& parsing_error : data->parsingErrors()) {
            qCDebug(ULogTrajectoryParserLog) << "ULog parsing error:" << QString::fromStdString(parsing_error);
        }
    }

    if (data->hadFatalError()) {
        errorMessage = QObject::tr("Could not parse ULog file");
        return false;
    }

    if (!data->isHeaderComplete()) {
        errorMessage = QObject::tr("Could not parse ULog header");
        return false;
    }

    const std::set<std::string> subscriptionNames = data->subscriptionNames();

    // Try vehicle_global_position first (preferred - has lat/lon in degrees)
    if (subscriptionNames.find("vehicle_global_position") != subscriptionNames.end()) {
        const std::shared_ptr<Subscription> subscription = data->subscription("vehicle_global_position");
        QGeoCoordinate lastCoord;
        int sampleCount = 0;

        for (const TypedDataView& sample : *subscription) {
            sampleCount++;
            if (_subsampleRate > 1 && sampleCount % _subsampleRate != 0) {
                continue;
            }

            try {
                TimestampedTrajectoryPoint point;

                // Timestamp is in microseconds
                uint64_t timestampUs = sample.at("timestamp").as<uint64_t>();
                point.timestamp = QDateTime::fromMSecsSinceEpoch(
                    static_cast<qint64>(timestampUs / 1000), QTimeZone::UTC);

                // Position - lat/lon in radians for vehicle_global_position
                double lat = sample.at("lat").as<double>();
                double lon = sample.at("lon").as<double>();
                float alt = sample.at("alt").as<float>();

                // Convert radians to degrees if needed (check magnitude)
                // vehicle_global_position stores lat/lon in radians
                if (qAbs(lat) <= 3.15 && qAbs(lon) <= 3.15) {
                    // Values look like radians, convert to degrees
                    lat = qRadiansToDegrees(lat);
                    lon = qRadiansToDegrees(lon);
                }

                point.coordinate = QGeoCoordinate(lat, lon, static_cast<double>(alt));

                // Distance filtering
                if (lastCoord.isValid() && lastCoord.distanceTo(point.coordinate) < _minDistance) {
                    continue;
                }
                lastCoord = point.coordinate;

                _points.append(point);
            } catch (const AccessException& e) {
                qCDebug(ULogTrajectoryParserLog) << "Field access error:" << e.what();
            }
        }

        qCDebug(ULogTrajectoryParserLog) << "Parsed" << _points.count()
                                         << "points from vehicle_global_position";
    }
    // Fallback to vehicle_gps_position
    else if (subscriptionNames.find("vehicle_gps_position") != subscriptionNames.end()) {
        const std::shared_ptr<Subscription> subscription = data->subscription("vehicle_gps_position");
        QGeoCoordinate lastCoord;
        int sampleCount = 0;

        for (const TypedDataView& sample : *subscription) {
            sampleCount++;
            if (_subsampleRate > 1 && sampleCount % _subsampleRate != 0) {
                continue;
            }

            try {
                TimestampedTrajectoryPoint point;

                uint64_t timestampUs = sample.at("timestamp").as<uint64_t>();
                point.timestamp = QDateTime::fromMSecsSinceEpoch(
                    static_cast<qint64>(timestampUs / 1000), QTimeZone::UTC);

                // vehicle_gps_position stores lat/lon in 1E-7 degrees
                int32_t lat1e7 = sample.at("lat").as<int32_t>();
                int32_t lon1e7 = sample.at("lon").as<int32_t>();
                int32_t altMm = sample.at("alt").as<int32_t>();

                double lat = lat1e7 / 1.0e7;
                double lon = lon1e7 / 1.0e7;
                double alt = altMm / 1000.0;

                point.coordinate = QGeoCoordinate(lat, lon, alt);

                // Distance filtering
                if (lastCoord.isValid() && lastCoord.distanceTo(point.coordinate) < _minDistance) {
                    continue;
                }
                lastCoord = point.coordinate;

                _points.append(point);
            } catch (const AccessException& e) {
                qCDebug(ULogTrajectoryParserLog) << "Field access error:" << e.what();
            }
        }

        qCDebug(ULogTrajectoryParserLog) << "Parsed" << _points.count()
                                         << "points from vehicle_gps_position";
    }

    if (_points.isEmpty()) {
        errorMessage = QObject::tr("No position data found in ULog file. "
                                   "Requires vehicle_global_position or vehicle_gps_position topic.");
        return false;
    }

    return true;
}

QDateTime ULogTrajectoryParser::startTime() const
{
    if (_points.isEmpty()) {
        return {};
    }
    return _points.first().timestamp;
}

QDateTime ULogTrajectoryParser::endTime() const
{
    if (_points.isEmpty()) {
        return {};
    }
    return _points.last().timestamp;
}
