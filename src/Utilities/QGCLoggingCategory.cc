/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCLoggingCategory.h"

#include <QtCore/QGlobalStatic>
#include <QtCore/QSettings>

static constexpr const char *kVideoAllLogCategory = "VideoAllLog";

// Add Global logging categories (not class specific) here using QGC_LOGGING_CATEGORY
QGC_LOGGING_CATEGORY(FirmwareUpgradeLog, "FirmwareUpgradeLog")
QGC_LOGGING_CATEGORY(FirmwareUpgradeVerboseLog, "FirmwareUpgradeVerboseLog")
QGC_LOGGING_CATEGORY(MissionCommandsLog, "MissionCommandsLog")
QGC_LOGGING_CATEGORY(GuidedActionsControllerLog, "GuidedActionsControllerLog")
QGC_LOGGING_CATEGORY(VideoAllLog, kVideoAllLogCategory)

Q_GLOBAL_STATIC(QGCLoggingCategoryRegister, _QGCLoggingCategoryRegisterInstance);

QGCLoggingCategoryRegister *QGCLoggingCategoryRegister::instance()
{
    return _QGCLoggingCategoryRegisterInstance();
}

QStringList QGCLoggingCategoryRegister::registeredCategories()
{
    _registeredCategories.sort();
    return _registeredCategories;
}

void QGCLoggingCategoryRegister::setCategoryLoggingOn(QStringView category, bool enable)
{
    QSettings settings;

    settings.beginGroup(_kFilterRulesSettingsGroup);
    settings.setValue(category, enable);

    settings.endGroup();
}

bool QGCLoggingCategoryRegister::categoryLoggingOn(QStringView category)
{
    QSettings settings;

    settings.beginGroup(_kFilterRulesSettingsGroup);
    return settings.value(category, false).toBool();
}

void QGCLoggingCategoryRegister::setFilterRulesFromSettings(const QString &commandLineLoggingOptions)
{
    if (!commandLineLoggingOptions.isEmpty()) {
        _commandLineLoggingOptions = commandLineLoggingOptions;
    }

    QString filterRules;
    filterRules += QStringLiteral("*Log.debug=false\n");
    filterRules += QStringLiteral("qgc.*.debug=false\n");

    static const QString filterRuleFormat = QStringLiteral("%1.debug=true\n");
    // Set up filters defined in settings
    bool videoAllLogSet = false;
    for (const QString &category : std::as_const(_registeredCategories)) {
        if (categoryLoggingOn(category)) {
            filterRules += filterRuleFormat.arg(category);
            if (category == kVideoAllLogCategory) {
                videoAllLogSet = true;
            }
        }
    }

    // Command line rules take precedence, so they go last in the list
    if (!_commandLineLoggingOptions.isEmpty()) {
        const QStringList logList = _commandLineLoggingOptions.split(",");

        if (logList[0] == QStringLiteral("full")) {
            filterRules += QStringLiteral("*Log.debug=true\n");
            for (const QString &log : logList) {
                filterRules += filterRuleFormat.arg(log);
            }
        } else {
            for (const QString &category: logList) {
                filterRules += filterRuleFormat.arg(category);
                if (category == kVideoAllLogCategory) {
                    videoAllLogSet = true;
                }
            }
        }
    }

    if (videoAllLogSet) {
        filterRules += filterRuleFormat.arg(QStringLiteral("qgc.videomanager.videomanager"));
        filterRules += filterRuleFormat.arg(QStringLiteral("qgc.videomanager.videoreceiver.gstreamer.gstvideoreceiver"));
        filterRules += filterRuleFormat.arg(QStringLiteral("qgc.videomanager.videoreceiver.gstreamer"));
    }

    // Logging from GStreamer library itself controlled by gstreamer debug levels is always turned on
    filterRules += filterRuleFormat.arg(QStringLiteral("qgc.videomanager.videoreceiver.gstreamer.api"));

    filterRules += QStringLiteral("qt.qml.connections=false");

    qDebug() << "Filter rules" << filterRules;
    QLoggingCategory::setFilterRules(filterRules);
}
