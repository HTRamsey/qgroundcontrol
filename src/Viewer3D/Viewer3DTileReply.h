/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QMetaType>
#include <QtCore/QObject>

Q_DECLARE_LOGGING_CATEGORY(Viewer3DTileReplyLog)

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

class Viewer3DTileReply : public QObject
{
    Q_OBJECT

public:
    explicit Viewer3DTileReply(int zoomLevel, int tileX, int tileY, int mapId, QObject *parent = nullptr);
    ~Viewer3DTileReply();

    struct tileInfo_t {
        int x, y, zoomLevel;
        QByteArray data;
        int mapId;
    };

signals:
    void tileDone(const tileInfo_t &tile);
    void tileEmpty(const tileInfo_t &tile);
    void tileError(const tileInfo_t &tile);
    void tileGiveUp(const tileInfo_t &tile);

private:
    void prepareDownload();
    void requestFinished();
    void requestError();
    void timeoutTimerEvent();

    QNetworkAccessManager *_networkManager = nullptr;
    QNetworkReply *_reply = nullptr;
    QTimer *_timeoutTimer = nullptr;
    tileInfo_t _tile;
    int _mapId = -1;
    int _timeoutCounter = -1;

    static QByteArray _bingNoTileImage;
};
Q_REGISTER_METATYPE(Viewer3DTileReply::tileInfo_t)
