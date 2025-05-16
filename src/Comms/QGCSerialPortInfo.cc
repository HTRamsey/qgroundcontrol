/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCSerialPortInfo.h"
#include "JsonHelper.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

QGC_LOGGING_CATEGORY(QGCSerialPortInfoLog, "qgc.comms.qgcserialportinfo")

bool QGCSerialPortInfo::_jsonLoaded = false;
bool QGCSerialPortInfo::_jsonDataValid = false;
QList<QGCSerialPortInfo::BoardInfo_t> QGCSerialPortInfo::_boardInfoList;
QList<QGCSerialPortInfo::BoardRegExpFallback_t> QGCSerialPortInfo::_boardDescriptionFallbackList;
QList<QGCSerialPortInfo::BoardRegExpFallback_t> QGCSerialPortInfo::_boardManufacturerFallbackList;

QGCSerialPortInfo::QGCSerialPortInfo()
    : QSerialPortInfo()
{
    qCDebug(QGCSerialPortInfoLog) << this;
}

QGCSerialPortInfo::QGCSerialPortInfo(const QSerialPort &port)
    : QSerialPortInfo(port)
{
    qCDebug(QGCSerialPortInfoLog) << this;
}

QGCSerialPortInfo::QGCSerialPortInfo(const QString &portName)
    : QSerialPortInfo(portName)
{
    qCDebug(QGCSerialPortInfoLog) << this;
}

QGCSerialPortInfo::~QGCSerialPortInfo()
{
    qCDebug(QGCSerialPortInfoLog) << this;
}

QString QGCSerialPortInfo::_boardTypeToString(BoardType boardType)
{
    switch (boardType) {
    case BoardType::BoardTypePixhawk:
        return QStringLiteral("Pixhawk");
    case BoardType::BoardTypeSiKRadio:
        return QStringLiteral("SiK Radio");
    case BoardType::BoardTypeOpenPilot:
        return QStringLiteral("OpenPilot");
    case BoardType::BoardTypeRTKGPS:
        return QStringLiteral("RTK GPS");
    case BoardType::BoardTypeUnknown:
    default:
        return QStringLiteral("Unknown");
    }
}

QGCSerialPortInfo::BoardType QGCSerialPortInfo::_boardClassStringToType(const QString &boardClass)
{
    struct BoardClassString2BoardType_t {
        const QString classString;
        const BoardType boardType = BoardType::BoardTypeUnknown;
    };

    static const QList<BoardClassString2BoardType_t> rgBoardClass2BoardType = {
        { _boardTypeToString(BoardType::BoardTypePixhawk), BoardType::BoardTypePixhawk },
        { _boardTypeToString(BoardType::BoardTypeRTKGPS), BoardType::BoardTypeRTKGPS },
        { _boardTypeToString(BoardType::BoardTypeSiKRadio), BoardType::BoardTypeSiKRadio },
        { _boardTypeToString(BoardType::BoardTypeOpenPilot), BoardType::BoardTypeOpenPilot },
    };

    for (const BoardClassString2BoardType_t &board : rgBoardClass2BoardType) {
        if (boardClass == board.classString) {
            return board.boardType;
        }
    }

    return BoardType::BoardTypeUnknown;
}

bool QGCSerialPortInfo::_loadJsonData()
{
    if (_jsonLoaded) {
        return _jsonDataValid;
    }

    _jsonLoaded = true;

    QString errorString;
    int version;
    const QJsonObject json = JsonHelper::openInternalQGCJsonFile(QStringLiteral(":/json/USBBoardInfo.json"), QString(_jsonFileTypeValue), 1, 1, version, errorString);
    if (!errorString.isEmpty()) {
        qCWarning(QGCSerialPortInfoLog) << "Internal Error:" << errorString;
        return false;
    }

    static const QList<JsonHelper::KeyValidateInfo> rootKeyInfoList = {
        { _jsonBoardInfoKey, QJsonValue::Array, true },
        { _jsonBoardDescriptionFallbackKey, QJsonValue::Array, true },
        { _jsonBoardManufacturerFallbackKey, QJsonValue::Array, true },
    };
    if (!JsonHelper::validateKeys(json, rootKeyInfoList, errorString)) {
        qCWarning(QGCSerialPortInfoLog) << errorString;
        return false;
    }

    static const QList<JsonHelper::KeyValidateInfo> boardKeyInfoList = {
        { _jsonVendorIDKey, QJsonValue::Double, true },
        { _jsonProductIDKey, QJsonValue::Double, true },
        { _jsonBoardClassKey, QJsonValue::String, true },
        { _jsonNameKey, QJsonValue::String, true },
    };
    const QJsonArray rgBoardInfo = json[_jsonBoardInfoKey].toArray();
    for (const QJsonValue &jsonValue : rgBoardInfo) {
        if (!jsonValue.isObject()) {
            qCWarning(QGCSerialPortInfoLog) << "Entry in boardInfo array is not object";
            return false;
        }

        const QJsonObject boardObject = jsonValue.toObject();
        if (!JsonHelper::validateKeys(boardObject, boardKeyInfoList, errorString)) {
            qCWarning(QGCSerialPortInfoLog) << errorString;
            return false;
        }

        const BoardInfo_t boardInfo = {
            boardObject[_jsonVendorIDKey].toInt(),
            boardObject[_jsonProductIDKey].toInt(),
            _boardClassStringToType(boardObject[_jsonBoardClassKey].toString()),
            boardObject[_jsonNameKey].toString()
        };
        if (boardInfo.boardType == BoardType::BoardTypeUnknown) {
            qCWarning(QGCSerialPortInfoLog) << "Bad board class" << boardObject[_jsonBoardClassKey].toString();
            return false;
        }

        _boardInfoList.append(boardInfo);
    }

    static const QList<JsonHelper::KeyValidateInfo> fallbackKeyInfoList = {
        { _jsonRegExpKey, QJsonValue::String, true },
        { _jsonBoardClassKey, QJsonValue::String, true },
        { _jsonAndroidOnlyKey, QJsonValue::Bool, false },
    };

    const QJsonArray rgBoardDescriptionFallback = json[_jsonBoardDescriptionFallbackKey].toArray();
    for (const QJsonValue &jsonValue : rgBoardDescriptionFallback) {
        if (!jsonValue.isObject()) {
            qCWarning(QGCSerialPortInfoLog) << "Entry in boardFallback array is not object";
            return false;
        }

        const QJsonObject fallbackObject = jsonValue.toObject();
        if (!JsonHelper::validateKeys(fallbackObject, fallbackKeyInfoList, errorString)) {
            qCWarning(QGCSerialPortInfoLog) << errorString;
            return false;
        }

        const BoardRegExpFallback_t boardFallback = {
            fallbackObject[_jsonRegExpKey].toString(),
            _boardClassStringToType(fallbackObject[_jsonBoardClassKey].toString()),
            fallbackObject[_jsonAndroidOnlyKey].toBool(false)
        };
        if (boardFallback.boardType == BoardType::BoardTypeUnknown) {
            qCWarning(QGCSerialPortInfoLog) << "Bad board class" << fallbackObject[_jsonBoardClassKey].toString();
            return false;
        }

        _boardDescriptionFallbackList.append(boardFallback);
    }

    const QJsonArray rgBoardManufacturerFallback = json[_jsonBoardManufacturerFallbackKey].toArray();
    for (const QJsonValue &jsonValue : rgBoardManufacturerFallback) {
        if (!jsonValue.isObject()) {
            qCWarning(QGCSerialPortInfoLog) << "Entry in boardFallback array is not object";
            return false;
        }

        const QJsonObject fallbackObject = jsonValue.toObject();
        if (!JsonHelper::validateKeys(fallbackObject, fallbackKeyInfoList, errorString)) {
            qCWarning(QGCSerialPortInfoLog) << errorString;
            return false;
        }

        const BoardRegExpFallback_t boardFallback = {
            fallbackObject[_jsonRegExpKey].toString(),
            _boardClassStringToType(fallbackObject[_jsonBoardClassKey].toString()),
            fallbackObject[_jsonAndroidOnlyKey].toBool(false)
        };
        if (boardFallback.boardType == BoardType::BoardTypeUnknown) {
            qCWarning(QGCSerialPortInfoLog) << "Bad board class" << fallbackObject[_jsonBoardClassKey].toString();
            return false;
        }

        _boardManufacturerFallbackList.append(boardFallback);
    }

    _jsonDataValid = true;

    return true;
}

bool QGCSerialPortInfo::getBoardInfo(QGCSerialPortInfo::BoardType &boardType, QString &name) const
{
    boardType = BoardType::BoardTypeUnknown;

    if (isNull()) {
        return false;
    }

    if (!_loadJsonData()) {
        return false;
    }

    for (const BoardInfo_t &boardInfo : _boardInfoList) {
        if ((vendorIdentifier() == boardInfo.vendorId) && ((productIdentifier() == boardInfo.productId) || (boardInfo.productId == 0))) {
            boardType = boardInfo.boardType;
            name = boardInfo.name;
            return true;
        }
    }

    Q_ASSERT(boardType == BoardType::BoardTypeUnknown);

    for (const BoardRegExpFallback_t &boardFallback : _boardDescriptionFallbackList) {
        if (description().contains(QRegularExpression(boardFallback.regExp, QRegularExpression::CaseInsensitiveOption))) {
#ifndef Q_OS_ANDROID
            if (boardFallback.androidOnly) {
                continue;
            }
#endif
            boardType = boardFallback.boardType;
            name = _boardTypeToString(boardType);
            return true;
        }
    }

    for (const BoardRegExpFallback_t &boardFallback : _boardManufacturerFallbackList) {
        if (manufacturer().contains(QRegularExpression(boardFallback.regExp, QRegularExpression::CaseInsensitiveOption))) {
#ifndef Q_OS_ANDROID
            if (boardFallback.androidOnly) {
                continue;
            }
#endif
            boardType = boardFallback.boardType;
            name = _boardTypeToString(boardType);
            return true;
        }
    }

    return false;
}

bool QGCSerialPortInfo::canFlash() const
{
    BoardType boardType;
    QString name;
    if (!getBoardInfo(boardType, name)) {
        return false;
    }

    static const QList<BoardType> flashable ={
        BoardType::BoardTypePixhawk,
        BoardType::BoardTypeSiKRadio
    };

    return flashable.contains(boardType);
}

bool QGCSerialPortInfo::isBootloader() const
{
    BoardType boardType;
    QString name;
    if (!getBoardInfo(boardType, name)) {
        return false;
    }

    return ((boardType == BoardType::BoardTypePixhawk) && description().contains(QStringLiteral("BL")));
}

bool QGCSerialPortInfo::isBlackCube() const
{
    return description().contains(QStringLiteral("CubeBlack"));
}

bool QGCSerialPortInfo::isSystemPort(const QSerialPortInfo &port)
{
#ifdef Q_OS_MACOS
    static const QStringList systemPortLocations = {
        QStringLiteral("tty.MALS"),
        QStringLiteral("tty.SOC"),
        QStringLiteral("tty.Bluetooth-Incoming-Port"),
        QStringLiteral("tty.usbserial"),
        QStringLiteral("tty.usbmodem")
    };
    for (const QString &systemPortLocation : systemPortLocations) {
        if (port.systemLocation().contains(systemPortLocation)) {
            return true;
        }
    }
#endif

    // TODO: Add Linux (LTE modems, etc) and Windows as needed

    return false;
}

QList<QGCSerialPortInfo> QGCSerialPortInfo::availablePorts()
{
    QList<QGCSerialPortInfo> list;

    const QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : availablePorts) {
        if (isSystemPort(portInfo)) {
            continue;
        }

        const QGCSerialPortInfo *const qgcPortInfo = reinterpret_cast<const QGCSerialPortInfo*>(&portInfo);
        list << *qgcPortInfo;
    }

    return list;
}
