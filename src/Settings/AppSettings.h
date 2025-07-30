/***************_qgcTranslatorSourceCode***********************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtQmlIntegration/QtQmlIntegration>

#include "SettingsGroup.h"

class AppSettings : public SettingsGroup
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_PROPERTY(QString crashSavePath            READ    crashSavePath           NOTIFY savePathsChanged)
    Q_PROPERTY(QString logSavePath              READ    logSavePath             NOTIFY savePathsChanged)
    Q_PROPERTY(QString mavlinkActionsSavePath   READ    mavlinkActionsSavePath  NOTIFY savePathsChanged)
    Q_PROPERTY(QString missionSavePath          READ    missionSavePath         NOTIFY savePathsChanged)
    Q_PROPERTY(QString parameterSavePath        READ    parameterSavePath       NOTIFY savePathsChanged)
    Q_PROPERTY(QString photoSavePath            READ    photoSavePath           NOTIFY savePathsChanged)
    Q_PROPERTY(QString telemetrySavePath        READ    telemetrySavePath       NOTIFY savePathsChanged)
    Q_PROPERTY(QString videoSavePath            READ    videoSavePath           NOTIFY savePathsChanged)
    Q_PROPERTY(QString kmlFileExtension         MEMBER  kmlFileExtension        CONSTANT)
    Q_PROPERTY(QString logFileExtension         MEMBER  logFileExtension        CONSTANT)
    Q_PROPERTY(QString missionFileExtension     MEMBER  missionFileExtension    CONSTANT)
    Q_PROPERTY(QString parameterFileExtension   MEMBER  parameterFileExtension  CONSTANT)
    Q_PROPERTY(QString planFileExtension        MEMBER  planFileExtension       CONSTANT)
    Q_PROPERTY(QString shpFileExtension         MEMBER  shpFileExtension        CONSTANT)
    Q_PROPERTY(QString telemetryFileExtension   MEMBER  telemetryFileExtension  CONSTANT)
    Q_PROPERTY(QString tilesetFileExtension     MEMBER  tilesetFileExtension    CONSTANT)
    Q_PROPERTY(QString waypointsFileExtension   MEMBER  waypointsFileExtension  CONSTANT)

    friend class QGCApplication;
public:
    explicit AppSettings(QObject *parent = nullptr);

    DEFINE_SETTING_NAME_GROUP()

    DEFINE_SETTINGFACT(androidSaveToSDCard)
    DEFINE_SETTINGFACT(appFontPointSize)
    DEFINE_SETTINGFACT(audioMuted)
    DEFINE_SETTINGFACT(batteryPercentRemainingAnnounce) // Important: This is only used to calculate battery swaps
    DEFINE_SETTINGFACT(customURL)
    DEFINE_SETTINGFACT(defaultMissionItemAltitude)
    DEFINE_SETTINGFACT(disableAllPersistence)
    DEFINE_SETTINGFACT(enableMultiVehiclePanel)
    DEFINE_SETTINGFACT(enforceChecklist)
    DEFINE_SETTINGFACT(esriToken)
    DEFINE_SETTINGFACT(firstRunPromptIdsShown)
    DEFINE_SETTINGFACT(followTarget)
    DEFINE_SETTINGFACT(gstDebugLevel)
    DEFINE_SETTINGFACT(indoorPalette)
    DEFINE_SETTINGFACT(loginAirLink)
    DEFINE_SETTINGFACT(mapboxAccount)
    DEFINE_SETTINGFACT(mapboxStyle)
    DEFINE_SETTINGFACT(mapboxToken)
    DEFINE_SETTINGFACT(offlineEditingAscentSpeed)
    DEFINE_SETTINGFACT(offlineEditingCruiseSpeed)
    DEFINE_SETTINGFACT(offlineEditingDescentSpeed)
    DEFINE_SETTINGFACT(offlineEditingFirmwareClass)
    DEFINE_SETTINGFACT(offlineEditingHoverSpeed)
    DEFINE_SETTINGFACT(offlineEditingVehicleClass)
    DEFINE_SETTINGFACT(passAirLink)
    DEFINE_SETTINGFACT(qLocaleLanguage)
    DEFINE_SETTINGFACT(savePath)
    DEFINE_SETTINGFACT(useChecklist)
    DEFINE_SETTINGFACT(virtualJoystick)
    DEFINE_SETTINGFACT(virtualJoystickAutoCenterThrottle)
    DEFINE_SETTINGFACT(virtualJoystickLeftHandedMode)
    DEFINE_SETTINGFACT(vworldToken)

    QString missionSavePath() const;
    QString parameterSavePath() const;
    QString telemetrySavePath() const;
    QString logSavePath() const;
    QString videoSavePath() const;
    QString photoSavePath() const;
    QString crashSavePath() const;
    QString mavlinkActionsSavePath() const;

    /// Helper methods for working with firstRunPromptIds QVariant settings string list
    Q_INVOKABLE void firstRunPromptIdsMarkIdAsShown(int id);
    static QList<int> firstRunPromptsIdsVariantToList(const QVariant &firstRunPromptIds);
    static QVariant firstRunPromptsIdsListToVariant(const QList<int> &rgIds);

    /// Application wide file extensions
    static constexpr const char *parameterFileExtension = "params";
    static constexpr const char *planFileExtension = "plan";
    static constexpr const char *missionFileExtension = "mission";
    static constexpr const char *waypointsFileExtension = "waypoints";
    static constexpr const char *fenceFileExtension = "fence";
    static constexpr const char *rallyPointFileExtension = "rally";
    static constexpr const char *telemetryFileExtension = "tlog";
    static constexpr const char *kmlFileExtension = "kml";
    static constexpr const char *shpFileExtension = "shp";
    static constexpr const char *logFileExtension = "ulg";
    static constexpr const char *tilesetFileExtension = "qgctiledb";

    /// Child directories of savePath for specific file types
    static constexpr const char *parameterDirectory = QT_TRANSLATE_NOOP("AppSettings", "Parameters");
    static constexpr const char *telemetryDirectory = QT_TRANSLATE_NOOP("AppSettings", "Telemetry");
    static constexpr const char *missionDirectory = QT_TRANSLATE_NOOP("AppSettings", "Missions");
    static constexpr const char *logDirectory = QT_TRANSLATE_NOOP("AppSettings", "Logs");
    static constexpr const char *videoDirectory = QT_TRANSLATE_NOOP("AppSettings", "Video");
    static constexpr const char *photoDirectory = QT_TRANSLATE_NOOP("AppSettings", "Photo");
    static constexpr const char *crashDirectory = QT_TRANSLATE_NOOP("AppSettings", "CrashLogs");
    static constexpr const char *mavlinkActionsDirectory = QT_TRANSLATE_NOOP("AppSettings", "MavlinkActions");

signals:
    void savePathsChanged();

private slots:
    void _indoorPaletteChanged();
    void _checkSavePathDirectories();
    void _qLocaleLanguageChanged();

private:
    /// Returns the current qLocaleLanguage setting bypassing the standard SettingsGroup path. It also validates
    /// that the value is a supported language. This should only be used by QGCApplication::setLanguage to query
    /// the language setting as early in the boot process as possible. Specfically prior to any JSON files being
    /// loaded such that JSON file can be translated. Also since this is a one-off mechanism custom build overrides
    /// for language are not currently supported.
    static QLocale::Language _qLocaleLanguageEarlyAccess();

    static QList<QLocale::Language> _rgReleaseLanguages;
    static QList<QLocale::Language> _rgPartialLanguages;

    struct LanguageInfo_t {
        QLocale::Language languageId;
        const char *languageName;
    };
    static LanguageInfo_t _rgLanguageInfo[];
};
