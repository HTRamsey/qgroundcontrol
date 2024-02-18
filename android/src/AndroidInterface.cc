/****************************************************************************
 *
 * Copyright (C) 2018 Pinecone Inc. All rights reserved.
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "AndroidInterface.h"

#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>

bool AndroidInterface::checkStoragePermissions()
{
    QString readPermission("android.permission.READ_EXTERNAL_STORAGE");
    QString writePermission("android.permission.WRITE_EXTERNAL_STORAGE");

    QStringList permissions = { readPermission, writePermission };
    for (const auto& permission: permissions) {
        auto futurePermissionResult = QtAndroidPrivate::checkPermission(permission);
        auto permissionResult = futurePermissionResult.result();
        if (permissionResult == QtAndroidPrivate::PermissionResult::Denied) {
            futurePermissionResult = QtAndroidPrivate::requestPermission(permission);
            permissionResult = futurePermissionResult.result();
            if (permissionResult == QtAndroidPrivate::PermissionResult::Denied) {
                return false;
            }
        }
    }

    return true;
}

QString AndroidInterface::getSDCardPath()
{
    if (!checkStoragePermissions()) {
        return QString();
    } else {
        auto value = QJniObject::callStaticObjectMethod(getQGCActivityClassName(), "getSDCardPath", "()Ljava/lang/String;");
        return value.toString();
    }
}

void AndroidInterface::setKeepScreenOn( bool bKeepScreenOn )
{
    Q_UNUSED( bKeepScreenOn );
}

const char * AndroidInterface::getQGCActivityClassName()
{
    return "org/mavlink/qgroundcontrol/QGCActivity";
}
