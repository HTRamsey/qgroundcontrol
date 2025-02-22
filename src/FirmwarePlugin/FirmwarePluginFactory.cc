/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "FirmwarePluginFactory.h"
#include "FirmwarePlugin.h"
#include "QGCLoggingCategory.h"

#include <QtCore/qapplicationstatic.h>

QGC_LOGGING_CATEGORY(FirmwarePluginFactoryLog, "qgc.firmwareplugin.firmwarepluginfactory");

/*===========================================================================*/

FirmwarePluginFactory::FirmwarePluginFactory(QObject *parent)
    : QObject(parent)
{
    // qCDebug(FirmwarePluginFactoryLog) << Q_FUNC_INFO << this;

    FirmwarePluginFactoryRegister::instance()->registerPluginFactory(this);
}

FirmwarePluginFactory::~FirmwarePluginFactory()
{
    // qCDebug(FirmwarePluginFactoryLog) << Q_FUNC_INFO << this;
}

/*===========================================================================*/

Q_APPLICATION_STATIC(FirmwarePluginFactoryRegister, _firmwarePluginFactoryRegisterInstance);

FirmwarePluginFactoryRegister *FirmwarePluginFactoryRegister::instance()
{
    return _firmwarePluginFactoryRegisterInstance();
}
