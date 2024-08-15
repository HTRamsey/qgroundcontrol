/****************************************************************************
 *
 * (c) 2009-2019 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 *   @brief Custom QGCCorePlugin Declaration
 *   @author Gus Grubba <gus@auterion.com>
 */

#pragma once

#include <QtCore/QTranslator>
#include <QtCore/QLoggingCategory>

#include "QGCCorePlugin.h"
#include "QGCOptions.h"

class CustomOptions;
class CustomPlugin;

Q_DECLARE_LOGGING_CATEGORY(CustomLog)

class CustomFlyViewOptions : public QGCFlyViewOptions
{
public:
    explicit CustomFlyViewOptions(CustomOptions *options, QObject *parent = nullptr);

    bool showMultiVehicleList() const final { return false; }
};

class CustomOptions : public QGCOptions
{
public:
    explicit CustomOptions(CustomPlugin *plugin, QObject *parent = nullptr);

    bool showFirmwareUpgrade() const final { return false; };
    QGCFlyViewOptions *flyViewOptions() final;

private:
    CustomFlyViewOptions *_flyViewOptions = nullptr;
};

class CustomPlugin : public QGCCorePlugin
{
    Q_OBJECT
public:
    CustomPlugin(QGCApplication *app, QGCToolbox *toolbox);
    ~CustomPlugin();

    QGCOptions *options() final;
    bool overrideSettingsGroupVisibility(QString name) final;
    bool adjustSettingMetaData(const QString &settingsGroup, FactMetaData &metaData) final;

    void setToolbox(QGCToolbox* toolbox);

private:
    CustomOptions *_options = nullptr;
};
