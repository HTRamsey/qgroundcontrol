/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "MavlinkCameraControl.h"
#include "QGCLoggingCategory.h"
#include "QmlObjectListModel.h"

QGC_LOGGING_CATEGORY(CameraControlLog, "qgc.camera.mavlinkcameracontrol")
QGC_LOGGING_CATEGORY(CameraControlVerboseLog, "qgc.camera.mavlinkcameracontrol:verbose")

MavlinkCameraControl::MavlinkCameraControl(QObject *parent)
    : FactGroup(0, parent, true /* ignore camel case */)
{
    // qCDebug(CameraControlLog) << this;
}

MavlinkCameraControl::~MavlinkCameraControl()
{
    // qCDebug(CameraControlLog) << this;
}

QString MavlinkCameraControl::captureImageStatusToStr(uint8_t imageStatus)
{
    switch (imageStatus) {
    case 0:
        return tr("Idle");
    case 1:
        return tr("Capturing");
    case 2:
        return tr("Idle: Interval set");
    case 3:
        return tr("Capturing: Interval set");
    default:
        return tr("Unknown");
    }
}

QString MavlinkCameraControl::captureVideoStatusToStr(uint8_t videoStatus)
{
    switch (videoStatus) {
    case 0:
        return tr("Idle");
    case 1:
        return tr("Capturing");
    default:
        return tr("Unknown");
    }
}

QString MavlinkCameraControl::storageStatusToStr(uint8_t status)
{
    switch (status) {
    case STORAGE_STATUS_EMPTY:
        return tr("Empty");
    case STORAGE_STATUS_UNFORMATTED:
        return tr("Unformatted");
    case STORAGE_STATUS_READY:
        return tr("Ready");
    case STORAGE_STATUS_NOT_SUPPORTED:
        return tr("Not Supported");
    default:
        return tr("Unknown");
    }
}

QString MavlinkCameraControl::cameraModeToStr(CameraMode mode)
{
    switch (mode) {
    case CAM_MODE_UNDEFINED:
        return tr("CAM_MODE_UNDEFINED");
    case CAM_MODE_PHOTO:
        return tr("CAM_MODE_PHOTO");
    case CAM_MODE_VIDEO:
        return tr("CAM_MODE_VIDEO");
    case CAM_MODE_SURVEY:
        return tr("CAM_MODE_SURVEY");
    default:
        return tr("Unknown");
    }
}
