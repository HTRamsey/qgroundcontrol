#pragma once

#include "GeoTagWorker.h"
#include <QtCore/QByteArray>

class ExifParser
{
public:
    ExifParser();
    ~ExifParser();
    double readTime(QByteArray& buf);
    bool write(QByteArray& buf, GeoTagWorker::cameraFeedbackPacket& geotag);
};
