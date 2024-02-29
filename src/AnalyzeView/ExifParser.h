#ifndef EXIFPARSER_H
#define EXIFPARSER_H

#include <QtCore/QDebug>

#include "GeoTagController.h"

class ExifParser
{
public:
    ExifParser();
    ~ExifParser();
    double readTime(QByteArray& buf);
    bool write(QByteArray& buf, GeoTagWorker::cameraFeedbackPacket& geotag);
};

#endif // EXIFPARSER_H
