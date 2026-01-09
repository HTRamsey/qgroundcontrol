#include "GeoFormatRegistry.h"
#include "KMLHelper.h"
#include "GPXHelper.h"
#include "GeoJsonHelper.h"
#include "SHPFileHelper.h"
#include "WKTHelper.h"
#include "OpenAirParser.h"
#include "GeoPackageHelper.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QLibrary>
#include <QtCore/QProcess>

QGC_LOGGING_CATEGORY(GeoFormatRegistryLog, "Utilities.Geo.GeoFormatRegistry")

namespace GeoFormatRegistry
{

namespace {

static bool s_gdalChecked = false;
static bool s_gdalAvailable = false;
static QString s_gdalVersion;

void checkGDAL()
{
    if (s_gdalChecked) {
        return;
    }
    s_gdalChecked = true;

    // Try to find ogr2ogr in PATH
    QProcess process;
    process.start(QStringLiteral("ogr2ogr"), QStringList() << QStringLiteral("--version"));
    if (process.waitForFinished(1000)) {
        QString output = QString::fromUtf8(process.readAllStandardOutput());
        if (output.contains(QStringLiteral("GDAL"))) {
            s_gdalAvailable = true;
            // Extract version: "GDAL 3.4.1, released 2021/12/27"
            int idx = output.indexOf(QStringLiteral("GDAL"));
            if (idx >= 0) {
                s_gdalVersion = output.mid(idx + 5).split(',').first().trimmed();
            }
            qCDebug(GeoFormatRegistryLog) << "GDAL available:" << s_gdalVersion;
        }
    }

#ifdef Q_OS_LINUX
    // Also try loading libgdal directly
    if (!s_gdalAvailable) {
        QLibrary gdal(QStringLiteral("gdal"));
        if (gdal.load()) {
            s_gdalAvailable = true;
            qCDebug(GeoFormatRegistryLog) << "GDAL library loaded directly";
            gdal.unload();
        }
    }
#endif

    if (!s_gdalAvailable) {
        qCDebug(GeoFormatRegistryLog) << "GDAL not available - using native parsers only";
    }
}

QList<FormatInfo> buildNativeFormats()
{
    QList<FormatInfo> formats;

    formats.append({
        QStringLiteral("KML"),
        QStringLiteral("Keyhole Markup Language"),
        {QStringLiteral("kml")},
        CanReadWrite,
        true
    });

    formats.append({
        QStringLiteral("KMZ"),
        QStringLiteral("Compressed KML"),
        {QStringLiteral("kmz")},
        CanRead | CanWritePolygons | CanWritePolylines,
        true
    });

    formats.append({
        QStringLiteral("GeoJSON"),
        QStringLiteral("GeoJSON"),
        {QStringLiteral("geojson"), QStringLiteral("json")},
        CanReadWrite,
        true
    });

    formats.append({
        QStringLiteral("GPX"),
        QStringLiteral("GPS Exchange Format"),
        {QStringLiteral("gpx")},
        CanReadWrite,
        true
    });

    formats.append({
        QStringLiteral("Shapefile"),
        QStringLiteral("ESRI Shapefile"),
        {QStringLiteral("shp")},
        CanReadWrite,
        true
    });

    formats.append({
        QStringLiteral("WKT"),
        QStringLiteral("Well-Known Text"),
        {QStringLiteral("wkt")},
        CanReadWrite,
        true
    });

    formats.append({
        QStringLiteral("OpenAir"),
        QStringLiteral("Airspace definition format"),
        {QStringLiteral("txt"), QStringLiteral("air")},
        CanReadPolygons | CanReadAirspace,
        true
    });

    formats.append({
        QStringLiteral("GeoPackage"),
        QStringLiteral("OGC GeoPackage"),
        {QStringLiteral("gpkg")},
        CanReadWrite,
        true
    });

    formats.append({
        QStringLiteral("CSV"),
        QStringLiteral("Comma-Separated Values"),
        {QStringLiteral("csv")},
        CanReadPoints | CanWritePoints,
        true
    });

    return formats;
}

} // anonymous namespace

QList<FormatInfo> supportedFormats()
{
    QList<FormatInfo> formats = buildNativeFormats();

    // Add GDAL formats if available
    if (isGDALAvailable()) {
        // These could be handled by GDAL if native fails
        formats.append({
            QStringLiteral("GML"),
            QStringLiteral("Geography Markup Language (via GDAL)"),
            {QStringLiteral("gml")},
            CanRead,
            false
        });

        formats.append({
            QStringLiteral("AIXM"),
            QStringLiteral("Aeronautical Information Exchange Model (via GDAL)"),
            {QStringLiteral("xml")},
            CanReadPolygons | CanReadAirspace,
            false
        });

        formats.append({
            QStringLiteral("MapInfo"),
            QStringLiteral("MapInfo TAB (via GDAL)"),
            {QStringLiteral("tab")},
            CanRead,
            false
        });
    }

    return formats;
}

QList<FormatInfo> nativeFormats()
{
    return buildNativeFormats();
}

bool isSupported(const QString &extension)
{
    QString ext = extension.toLower();
    QList<FormatInfo> formats = supportedFormats();

    for (const FormatInfo &format : formats) {
        if (format.extensions.contains(ext)) {
            return true;
        }
    }

    return false;
}

FormatInfo formatInfo(const QString &extension)
{
    QString ext = extension.toLower();
    QList<FormatInfo> formats = supportedFormats();

    for (const FormatInfo &format : formats) {
        if (format.extensions.contains(ext)) {
            return format;
        }
    }

    return FormatInfo();
}

QString readFileFilter()
{
    QStringList allExtensions;
    QStringList filters;

    QList<FormatInfo> formats = supportedFormats();
    for (const FormatInfo &format : formats) {
        if (format.capabilities & CanRead) {
            QStringList exts;
            for (const QString &ext : format.extensions) {
                exts.append(QStringLiteral("*.%1").arg(ext));
                allExtensions.append(QStringLiteral("*.%1").arg(ext));
            }
            filters.append(QStringLiteral("%1 (%2)").arg(format.name, exts.join(' ')));
        }
    }

    // Add "All Geo Files" at the beginning
    QString allFilter = QStringLiteral("All Geo Files (%1)").arg(allExtensions.join(' '));
    filters.prepend(allFilter);

    return filters.join(QStringLiteral(";;"));
}

QString writeFileFilter()
{
    QStringList filters;

    QList<FormatInfo> formats = supportedFormats();
    for (const FormatInfo &format : formats) {
        if (format.capabilities & CanWrite) {
            QStringList exts;
            for (const QString &ext : format.extensions) {
                exts.append(QStringLiteral("*.%1").arg(ext));
            }
            filters.append(QStringLiteral("%1 (%2)").arg(format.name, exts.join(' ')));
        }
    }

    return filters.join(QStringLiteral(";;"));
}

LoadResult loadFile(const QString &filePath)
{
    LoadResult result;

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        result.errorString = QObject::tr("File not found: %1").arg(filePath);
        return result;
    }

    QString ext = fileInfo.suffix().toLower();

    // Try native loaders first
    if (ext == QStringLiteral("kml")) {
        QString errorString;
        KMLHelper::loadPointsFromFile(filePath, result.points, errorString);
        KMLHelper::loadPolylinesFromFile(filePath, result.polylines, errorString, true);
        KMLHelper::loadPolygonsFromFile(filePath, result.polygons, errorString, true);
        result.formatUsed = QStringLiteral("KML");
        result.success = result.totalFeatures() > 0;
        if (!result.success) {
            result.errorString = errorString;
        }
        return result;
    }

    if (ext == QStringLiteral("geojson") || ext == QStringLiteral("json")) {
        QString errorString;
        GeoJsonHelper::loadPointsFromFile(filePath, result.points, errorString);
        GeoJsonHelper::loadPolylinesFromFile(filePath, result.polylines, errorString, true);
        GeoJsonHelper::loadPolygonsFromFile(filePath, result.polygons, errorString, true);
        result.formatUsed = QStringLiteral("GeoJSON");
        result.success = result.totalFeatures() > 0;
        if (!result.success) {
            result.errorString = errorString;
        }
        return result;
    }

    if (ext == QStringLiteral("gpx")) {
        QString errorString;
        GPXHelper::loadPointsFromFile(filePath, result.points, errorString);
        GPXHelper::loadPolylinesFromFile(filePath, result.polylines, errorString, true);
        GPXHelper::loadPolygonsFromFile(filePath, result.polygons, errorString, true);
        result.formatUsed = QStringLiteral("GPX");
        result.success = result.totalFeatures() > 0;
        if (!result.success) {
            result.errorString = errorString;
        }
        return result;
    }

    if (ext == QStringLiteral("shp")) {
        QString errorString;
        SHPFileHelper::loadPointsFromFile(filePath, result.points, errorString);
        SHPFileHelper::loadPolylinesFromFile(filePath, result.polylines, errorString, true);
        SHPFileHelper::loadPolygonsFromFile(filePath, result.polygons, errorString, true);
        result.formatUsed = QStringLiteral("Shapefile");
        result.success = result.totalFeatures() > 0;
        if (!result.success) {
            result.errorString = errorString;
        }
        return result;
    }

    if (ext == QStringLiteral("gpkg")) {
        GeoPackageHelper::LoadResult gpkgResult = GeoPackageHelper::loadAllFeatures(filePath);
        result.points = gpkgResult.points;
        result.polylines = gpkgResult.polylines;
        result.polygons = gpkgResult.polygons;
        result.formatUsed = QStringLiteral("GeoPackage");
        result.success = gpkgResult.success;
        result.errorString = gpkgResult.errorString;
        return result;
    }

    if (ext == QStringLiteral("txt") || ext == QStringLiteral("air")) {
        // Try OpenAir format
        OpenAirParser::ParseResult airResult = OpenAirParser::parseFile(filePath);
        if (airResult.success) {
            for (const OpenAirParser::Airspace &airspace : airResult.airspaces) {
                result.polygons.append(airspace.boundary);
            }
            result.formatUsed = QStringLiteral("OpenAir");
            result.success = true;
            return result;
        }
        // If OpenAir fails, don't report error - might be a regular text file
    }

    if (ext == QStringLiteral("wkt")) {
        // Read WKT from file
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString content = QString::fromUtf8(file.readAll());
            QString errorString;

            // Try different WKT types
            QList<QGeoCoordinate> polygon;
            if (WKTHelper::parsePolygon(content, polygon, errorString)) {
                result.polygons.append(polygon);
            }

            QList<QGeoCoordinate> line;
            if (WKTHelper::parseLineString(content, line, errorString)) {
                result.polylines.append(line);
            }

            QGeoCoordinate point;
            if (WKTHelper::parsePoint(content, point, errorString)) {
                result.points.append(point);
            }

            result.formatUsed = QStringLiteral("WKT");
            result.success = result.totalFeatures() > 0;
            if (!result.success) {
                result.errorString = errorString;
            }
            return result;
        }
    }

    // Try GDAL as fallback
    if (isGDALAvailable()) {
        return loadWithGDAL(filePath);
    }

    result.errorString = QObject::tr("Unsupported format: %1").arg(ext);
    return result;
}

bool loadPoints(const QString &filePath, QList<QGeoCoordinate> &points, QString &errorString)
{
    LoadResult result = loadFile(filePath);
    if (!result.success) {
        errorString = result.errorString;
        return false;
    }
    points = result.points;
    return true;
}

bool loadPolylines(const QString &filePath, QList<QList<QGeoCoordinate>> &polylines, QString &errorString)
{
    LoadResult result = loadFile(filePath);
    if (!result.success) {
        errorString = result.errorString;
        return false;
    }
    polylines = result.polylines;
    return true;
}

bool loadPolygons(const QString &filePath, QList<QList<QGeoCoordinate>> &polygons, QString &errorString)
{
    LoadResult result = loadFile(filePath);
    if (!result.success) {
        errorString = result.errorString;
        return false;
    }
    polygons = result.polygons;
    return true;
}

// ============================================================================
// Save Functions
// ============================================================================

SaveResult savePoint(const QString &filePath, const QGeoCoordinate &point)
{
    QList<QGeoCoordinate> points;
    points.append(point);
    return savePoints(filePath, points);
}

SaveResult savePoints(const QString &filePath, const QList<QGeoCoordinate> &points)
{
    SaveResult result;

    QFileInfo fileInfo(filePath);
    QString ext = fileInfo.suffix().toLower();

    QString errorString;

    if (ext == QStringLiteral("kml")) {
        if (KMLHelper::savePointsToFile(filePath, points, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("KML");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("geojson") || ext == QStringLiteral("json")) {
        if (GeoJsonHelper::savePointsToFile(filePath, points, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("GeoJSON");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("gpx")) {
        if (GPXHelper::savePointsToFile(filePath, points, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("GPX");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("shp")) {
        if (SHPFileHelper::savePointsToFile(filePath, points, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("Shapefile");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("gpkg")) {
        if (GeoPackageHelper::savePoints(filePath, points, QStringLiteral("points"), errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("GeoPackage");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("wkt")) {
        if (WKTHelper::savePointsToFile(filePath, points, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("WKT");
        } else {
            result.errorString = errorString;
        }
    } else {
        result.errorString = QObject::tr("Unsupported format for point save: %1").arg(ext);
    }

    return result;
}

SaveResult savePolyline(const QString &filePath, const QList<QGeoCoordinate> &polyline)
{
    QList<QList<QGeoCoordinate>> polylines;
    polylines.append(polyline);
    return savePolylines(filePath, polylines);
}

SaveResult savePolylines(const QString &filePath, const QList<QList<QGeoCoordinate>> &polylines)
{
    SaveResult result;

    QFileInfo fileInfo(filePath);
    QString ext = fileInfo.suffix().toLower();

    QString errorString;

    if (ext == QStringLiteral("kml")) {
        if (KMLHelper::savePolylinesToFile(filePath, polylines, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("KML");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("geojson") || ext == QStringLiteral("json")) {
        if (GeoJsonHelper::savePolylinesToFile(filePath, polylines, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("GeoJSON");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("gpx")) {
        if (GPXHelper::savePolylinesToFile(filePath, polylines, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("GPX");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("shp")) {
        if (SHPFileHelper::savePolylinesToFile(filePath, polylines, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("Shapefile");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("gpkg")) {
        if (GeoPackageHelper::savePolylines(filePath, polylines, QStringLiteral("polylines"), errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("GeoPackage");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("wkt")) {
        if (WKTHelper::savePolylinesToFile(filePath, polylines, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("WKT");
        } else {
            result.errorString = errorString;
        }
    } else {
        result.errorString = QObject::tr("Unsupported format for polyline save: %1").arg(ext);
    }

    return result;
}

SaveResult savePolygon(const QString &filePath, const QList<QGeoCoordinate> &polygon)
{
    QList<QList<QGeoCoordinate>> polygons;
    polygons.append(polygon);
    return savePolygons(filePath, polygons);
}

SaveResult savePolygons(const QString &filePath, const QList<QList<QGeoCoordinate>> &polygons)
{
    SaveResult result;

    QFileInfo fileInfo(filePath);
    QString ext = fileInfo.suffix().toLower();

    QString errorString;

    if (ext == QStringLiteral("kml")) {
        if (KMLHelper::savePolygonsToFile(filePath, polygons, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("KML");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("geojson") || ext == QStringLiteral("json")) {
        if (GeoJsonHelper::savePolygonsToFile(filePath, polygons, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("GeoJSON");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("gpx")) {
        if (GPXHelper::savePolygonsToFile(filePath, polygons, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("GPX");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("shp")) {
        if (SHPFileHelper::savePolygonsToFile(filePath, polygons, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("Shapefile");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("gpkg")) {
        if (GeoPackageHelper::savePolygons(filePath, polygons, QStringLiteral("polygons"), errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("GeoPackage");
        } else {
            result.errorString = errorString;
        }
    } else if (ext == QStringLiteral("wkt")) {
        if (WKTHelper::savePolygonsToFile(filePath, polygons, errorString)) {
            result.success = true;
            result.formatUsed = QStringLiteral("WKT");
        } else {
            result.errorString = errorString;
        }
    } else {
        result.errorString = QObject::tr("Unsupported format for polygon save: %1").arg(ext);
    }

    return result;
}

// ============================================================================
// Validation Functions
// ============================================================================

namespace {

// Check which save implementations exist for a format by extension
Capabilities checkSaveImplementations(const QString &ext)
{
    Capabilities caps;

    // These are the formats that have save implementations in the save functions above
    if (ext == QStringLiteral("kml") || ext == QStringLiteral("geojson") ||
        ext == QStringLiteral("json") || ext == QStringLiteral("gpx") ||
        ext == QStringLiteral("shp") || ext == QStringLiteral("gpkg") ||
        ext == QStringLiteral("wkt")) {
        caps = CanWritePoints | CanWritePolylines | CanWritePolygons;
    }
    // KMZ has partial write support (no points)
    else if (ext == QStringLiteral("kmz")) {
        caps = CanWritePolylines | CanWritePolygons;
    }
    // CSV only supports points
    else if (ext == QStringLiteral("csv")) {
        caps = CanWritePoints;
    }

    return caps;
}

// Check which load implementations exist for a format by extension
Capabilities checkLoadImplementations(const QString &ext)
{
    Capabilities caps;

    if (ext == QStringLiteral("kml") || ext == QStringLiteral("kmz") ||
        ext == QStringLiteral("geojson") || ext == QStringLiteral("json") ||
        ext == QStringLiteral("gpx") || ext == QStringLiteral("shp") ||
        ext == QStringLiteral("gpkg") || ext == QStringLiteral("wkt")) {
        caps = CanReadPoints | CanReadPolylines | CanReadPolygons;
    }
    // OpenAir only has polygon/airspace support
    else if (ext == QStringLiteral("txt") || ext == QStringLiteral("air")) {
        caps = CanReadPolygons | CanReadAirspace;
    }
    // CSV only supports points
    else if (ext == QStringLiteral("csv")) {
        caps = CanReadPoints;
    }

    return caps;
}

} // anonymous namespace

QList<ValidationResult> validateCapabilities()
{
    QList<ValidationResult> results;

    QList<FormatInfo> formats = nativeFormats();  // Only validate native formats

    for (const FormatInfo &format : formats) {
        ValidationResult result;
        result.formatName = format.name;

        for (const QString &ext : format.extensions) {
            Capabilities actualLoad = checkLoadImplementations(ext);
            Capabilities actualSave = checkSaveImplementations(ext);
            Capabilities actualCaps = actualLoad | actualSave;

            // Check read capabilities
            if ((format.capabilities & CanReadPoints) && !(actualCaps & CanReadPoints)) {
                result.issues.append(QStringLiteral("Claims CanReadPoints but no implementation for .%1").arg(ext));
            }
            if ((format.capabilities & CanReadPolylines) && !(actualCaps & CanReadPolylines)) {
                result.issues.append(QStringLiteral("Claims CanReadPolylines but no implementation for .%1").arg(ext));
            }
            if ((format.capabilities & CanReadPolygons) && !(actualCaps & CanReadPolygons)) {
                result.issues.append(QStringLiteral("Claims CanReadPolygons but no implementation for .%1").arg(ext));
            }

            // Check write capabilities
            if ((format.capabilities & CanWritePoints) && !(actualCaps & CanWritePoints)) {
                result.issues.append(QStringLiteral("Claims CanWritePoints but no implementation for .%1").arg(ext));
            }
            if ((format.capabilities & CanWritePolylines) && !(actualCaps & CanWritePolylines)) {
                result.issues.append(QStringLiteral("Claims CanWritePolylines but no implementation for .%1").arg(ext));
            }
            if ((format.capabilities & CanWritePolygons) && !(actualCaps & CanWritePolygons)) {
                result.issues.append(QStringLiteral("Claims CanWritePolygons but no implementation for .%1").arg(ext));
            }
        }

        result.valid = result.issues.isEmpty();
        if (!result.valid) {
            qCWarning(GeoFormatRegistryLog) << "Format" << format.name << "has capability mismatches:" << result.issues;
        }
        results.append(result);
    }

    return results;
}

bool allCapabilitiesValid()
{
    QList<ValidationResult> results = validateCapabilities();
    for (const ValidationResult &result : results) {
        if (!result.valid) {
            return false;
        }
    }
    return true;
}

bool isGDALAvailable()
{
    checkGDAL();
    return s_gdalAvailable;
}

QString gdalVersion()
{
    checkGDAL();
    return s_gdalVersion;
}

LoadResult loadWithGDAL(const QString &filePath)
{
    LoadResult result;

    if (!isGDALAvailable()) {
        result.errorString = QObject::tr("GDAL not available");
        return result;
    }

    // Convert to GeoJSON using ogr2ogr, then parse with our GeoJSON loader
    QString tempFile = QStringLiteral("/tmp/qgc_gdal_temp_%1.geojson").arg(QDateTime::currentMSecsSinceEpoch());

    QProcess process;
    process.start(QStringLiteral("ogr2ogr"),
                  QStringList() << QStringLiteral("-f") << QStringLiteral("GeoJSON")
                                << tempFile << filePath);

    if (!process.waitForFinished(30000)) {
        result.errorString = QObject::tr("GDAL conversion timed out");
        return result;
    }

    if (process.exitCode() != 0) {
        result.errorString = QObject::tr("GDAL conversion failed: %1")
                                 .arg(QString::fromUtf8(process.readAllStandardError()));
        return result;
    }

    // Load the converted GeoJSON
    QString errorString;
    GeoJsonHelper::loadPointsFromFile(tempFile, result.points, errorString);
    GeoJsonHelper::loadPolylinesFromFile(tempFile, result.polylines, errorString, true);
    GeoJsonHelper::loadPolygonsFromFile(tempFile, result.polygons, errorString, true);

    // Clean up temp file
    QFile::remove(tempFile);

    result.formatUsed = QStringLiteral("GDAL");
    result.success = result.totalFeatures() > 0;
    if (!result.success) {
        result.errorString = errorString;
    }

    return result;
}

QStringList gdalFormats()
{
    if (!isGDALAvailable()) {
        return QStringList();
    }

    // Get GDAL supported formats
    QProcess process;
    process.start(QStringLiteral("ogr2ogr"), QStringList() << QStringLiteral("--formats"));

    if (!process.waitForFinished(5000)) {
        return QStringList();
    }

    QStringList formats;
    QString output = QString::fromUtf8(process.readAllStandardOutput());
    QStringList lines = output.split('\n');

    for (const QString &line : lines) {
        if (line.contains(QStringLiteral("->"))) {
            QString format = line.split(QStringLiteral("->")).first().trimmed();
            format = format.split(QStringLiteral(" ")).first();
            if (!format.isEmpty()) {
                formats.append(format);
            }
        }
    }

    return formats;
}

} // namespace GeoFormatRegistry
