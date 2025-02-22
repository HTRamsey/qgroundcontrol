/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "SettingsFact.h"
#include "QGCApplication.h"
#include "QGCCorePlugin.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QSettings>
#include <QtQml/QQmlEngine>

QGC_LOGGING_CATEGORY(SettingsFactLog, "qgc.factsystem.settingsfact");

SettingsFact::SettingsFact(QObject *parent)
    : Fact(parent)
{
    // qCDebug(SettingsFactLog) << Q_FUNC_INFO << this;
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

SettingsFact::SettingsFact(const QString &settingsGroup, FactMetaData *metaData, QObject *parent)
    : Fact(0, metaData->name(), metaData->type(), parent)
    , _settingsGroup(settingsGroup)
{
    // qCDebug(SettingsFactLog) << Q_FUNC_INFO << this;
    QSettings settings;

    if (!_settingsGroup.isEmpty()) {
        settings.beginGroup(_settingsGroup);
    }

    // Allow core plugin a chance to override the default value
    _visible = QGCCorePlugin::instance()->adjustSettingMetaData(settingsGroup, *metaData);
    setMetaData(metaData);

    if (metaData->defaultValueAvailable()) {
        const QVariant rawDefaultValue = metaData->rawDefaultValue();
        if (qgcApp()->runningUnitTests()) {
            // Don't use saved settings
            _rawValue = rawDefaultValue;
        } else {
            if (_visible) {
                QVariant typedValue;
                QString errorString;
                metaData->convertAndValidateRaw(settings.value(_name, rawDefaultValue), true /* convertOnly */, typedValue, errorString);
                _rawValue = typedValue;
            } else {
                // Setting is not visible, force to default value always
                settings.setValue(_name, rawDefaultValue);
                _rawValue = rawDefaultValue;
            }
        }
    }

    (void) connect(this, &Fact::rawValueChanged, this, &SettingsFact::_rawValueChanged);
}

SettingsFact::SettingsFact(const SettingsFact &other, QObject *parent)
    : Fact(other, parent)
{
    // qCDebug(AudioOutputLog) << Q_FUNC_INFO << this;
    *this = other;
}

SettingsFact::~SettingsFact()
{
    // qCDebug(AudioOutputLog) << Q_FUNC_INFO << this;
}

const SettingsFact &SettingsFact::operator=(const SettingsFact &other)
{
    Fact::operator=(other);

    _settingsGroup = other._settingsGroup;

    return *this;
}

void SettingsFact::_rawValueChanged(QVariant value)
{
    QSettings settings;

    if (!_settingsGroup.isEmpty()) {
        settings.beginGroup(_settingsGroup);
    }

    settings.setValue(_name, value);
}
