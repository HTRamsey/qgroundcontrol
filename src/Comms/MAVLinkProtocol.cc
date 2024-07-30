/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "MAVLinkProtocol.h"
#include "LinkManager.h"
#include "LinkInterface.h"
#include "QGCApplication.h"
#include "QGCToolbox.h"
#include "MultiVehicleManager.h"
#include "SettingsManager.h"
#include "QGCTemporaryFile.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QMetaType>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <QtQml/QtQml>

QGC_LOGGING_CATEGORY(MAVLinkProtocolLog, "qgc.comms.mavlinkprotocol")

Q_APPLICATION_STATIC(MAVLinkProtocol, s_mavlinkProtocol);

MAVLinkProtocol* MAVLinkProtocol::instance()
{
    return s_mavlinkProtocol();
}

MAVLinkProtocol::MAVLinkProtocol(QObject *parent)
    : QObject(parent)
    , m_tempLogFile(new QGCTemporaryFile(QString("%2.%3").arg(s_tempLogFileTemplate, s_logFileExtension)))
{
    // qCDebug(MAVLinkProtocolLog) << Q_FUNC_INFO << this;

    (void) memset(m_totalReceiveCounter, 0, sizeof(m_totalReceiveCounter));
    (void) memset(m_totalLossCounter, 0, sizeof(m_totalLossCounter));
    (void) memset(m_runningLossPercent, 0, sizeof(m_runningLossPercent));
    (void) memset(m_firstMessage, 1, sizeof(m_firstMessage));
    (void) memset(&m_status, 0, sizeof(m_status));
    (void) memset(&m_message, 0, sizeof(m_message));
}

MAVLinkProtocol::~MAVLinkProtocol()
{
    storeSettings();
    _closeLogFile();

    // qCDebug(MAVLinkProtocolLog) << Q_FUNC_INFO << this;
}

void MAVLinkProtocol::setVersion(uint16_t version)
{
    const QList<SharedLinkInterfacePtr> sharedLinks = qgcApp()->toolbox()->linkManager()->links();
    for (const SharedLinkInterfacePtr &interface : sharedLinks) {
        mavlink_set_proto_version(interface.get()->mavlinkChannel());
    }

    m_currentVersion = version;
}

/*void MAVLinkProtocol::setToolbox(QGCToolbox *toolbox)
{
   QGCTool::setToolbox(toolbox);

   loadSettings();

   (void) connect(this, &MAVLinkProtocol::protocolStatusMessage, qgcApp(), &QGCApplication::criticalMessageBoxOnMainThread);
   (void) connect(this, &MAVLinkProtocol::saveTelemetryLog, qgcApp(), &QGCApplication::saveTelemetryLogOnMainThread);
   (void) connect(this, &MAVLinkProtocol::checkTelemetrySavePath, qgcApp(), &QGCApplication::checkTelemetrySavePathOnMainThread);

   (void) connect(qgcApp()->toolbox()->multiVehicleManager(), &MultiVehicleManager::vehicleAdded, this, &MAVLinkProtocol::_vehicleCountChanged);
   (void) connect(qgcApp()->toolbox()->multiVehicleManager(), &MultiVehicleManager::vehicleRemoved, this, &MAVLinkProtocol::_vehicleCountChanged);

   enableVersionCheck(true);
}*/

void MAVLinkProtocol::loadSettings()
{
    QSettings settings;
    settings.beginGroup("QGC_MAVLINK_PROTOCOL");
    enableVersionCheck(settings.value("VERSION_CHECK_ENABLED", versionCheckEnabled()).toBool());

    bool ok = false;
    const uint temp = settings.value("GCS_SYSTEM_ID", getSystemId()).toUInt(&ok);
    if (ok && (temp > 0) && (temp < 256)) {
        setSystemId(temp);
    }
}

void MAVLinkProtocol::storeSettings()
{
    QSettings settings;
    settings.beginGroup("QGC_MAVLINK_PROTOCOL");
    settings.setValue("VERSION_CHECK_ENABLED", versionCheckEnabled());
    settings.setValue("GCS_SYSTEM_ID", getSystemId());
}

void MAVLinkProtocol::resetMetadataForLink(LinkInterface *link)
{
    const uint8_t channel = link->mavlinkChannel();
    m_totalReceiveCounter[channel] = 0;
    m_totalLossCounter[channel] = 0;
    m_runningLossPercent[channel] = 0.f;
    for (uint16_t i = 0; i < 256; i++) {
        m_firstMessage[channel][i] = 1;
    }
    link->setDecodedFirstMavlinkPacket(false);
}

void MAVLinkProtocol::logSentBytes(LinkInterface *link, const QByteArray &data)
{
    Q_UNUSED(link);

    uint8_t bytes_time[sizeof(qint64)];
    if (!m_logSuspendError && !m_logSuspendReplay && m_tempLogFile->isOpen()) {
        const qint64 time = QDateTime::currentMSecsSinceEpoch() * 1000;
        qToBigEndian(time, bytes_time);
        (void) data.insert(0, QByteArray(reinterpret_cast<const char*>(bytes_time), sizeof(bytes_time)));

        const qsizetype len = data.length();
        if (m_tempLogFile->write(data) != data.length()) {
            const QString message = QStringLiteral("MAVLink Logging failed. Could not write to file %1, logging disabled.").arg(m_tempLogFile->fileName());
            emit protocolStatusMessage(getName(), message);
            _stopLogging();
            m_logSuspendError = true;
        }
    }

}

void MAVLinkProtocol::receiveBytes(LinkInterface *link, const QByteArray &data)
{
    const SharedLinkInterfacePtr linkPtr = qgcApp()->toolbox()->linkManager()->sharedLinkInterfacePointerForLink(link, true);
    if (!linkPtr) {
        qCDebug(MAVLinkProtocolLog) << "receiveBytes: link gone!" << data.size() << " bytes arrived too late";
        return;
    }

    const uint8_t mavlinkChannel = link->mavlinkChannel();
    for (int position = 0; position < data.size(); position++) {
        if (mavlink_parse_char(mavlinkChannel, static_cast<uint8_t>(data[position]), &m_message, &m_status)) {
            if (!link->decodedFirstMavlinkPacket()) {
                link->setDecodedFirstMavlinkPacket(true);
                mavlink_status_t* const mavlinkStatus = mavlink_get_channel_status(mavlinkChannel);
                if ((!(mavlinkStatus->flags & MAVLINK_STATUS_FLAG_IN_MAVLINK1)) && (mavlinkStatus->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1)) {
                    qCDebug(MAVLinkProtocolLog) << QStringLiteral("Switching outbound to mavlink 2.0 due to incoming mavlink 2.0 packet:") << mavlinkChannel;
                    mavlinkStatus->flags &= ~MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
                    setVersion(2);
                }
            }

            //-----------------------------------------------------------------
            // MAVLink Status
            uint8_t lastSeq = m_lastIndex[m_message.sysid][m_message.compid];
            uint8_t expectedSeq = lastSeq + 1;
            m_totalReceiveCounter[mavlinkChannel]++;
            if (m_firstMessage[m_message.sysid][m_message.compid] != 0) {
                m_firstMessage[m_message.sysid][m_message.compid] = 0;
                lastSeq = m_message.seq;
                expectedSeq = m_message.seq;
            }

            if (m_message.seq != expectedSeq)
            {
                uint64_t lostMessages = m_message.seq;
                if (lostMessages < expectedSeq) {
                    lostMessages += 255;
                }
                lostMessages -= expectedSeq;
                m_totalLossCounter[mavlinkChannel] += lostMessages;
            }

            m_lastIndex[m_message.sysid][m_message.compid] = m_message.seq;
            const uint64_t totalSent = m_totalReceiveCounter[mavlinkChannel] + m_totalLossCounter[mavlinkChannel];
            float receiveLossPercent = static_cast<float>(static_cast<double>(m_totalLossCounter[mavlinkChannel]) / static_cast<double>(totalSent));
            receiveLossPercent *= 100.0f;
            receiveLossPercent *= 0.5f;
            receiveLossPercent += (m_runningLossPercent[mavlinkChannel] * 0.5f);
            m_runningLossPercent[mavlinkChannel] = receiveLossPercent;

            //-----------------------------------------------------------------
            // MAVLink forwarding
            const bool forwardingEnabled = qgcApp()->toolbox()->settingsManager()->appSettings()->forwardMavlink()->rawValue().toBool();
            if (m_message.msgid == MAVLINK_MSG_ID_SETUP_SIGNING) {
                forwardingEnabled = false;
            }
            if (forwardingEnabled) {
                SharedLinkInterfacePtr forwardingLink = qgcApp()->toolbox()->linkManager()->mavlinkForwardingLink();
                if (forwardingLink) {
                    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
                    const uint16_t len = mavlink_msg_to_send_buffer(buf, &m_message);
                    forwardingLink->writeBytesThreadSafe(reinterpret_cast<const char*>(buf), len);
                }
            }

            //-----------------------------------------------------------------
            // MAVLink forwarding support
            const bool forwardingSupportEnabled = qgcApp()->toolbox()->linkManager()->mavlinkSupportForwardingEnabled();
            if (m_message.msgid == MAVLINK_MSG_ID_SETUP_SIGNING) {
                forwardingSupportEnabled = false;
            }
            if (forwardingSupportEnabled) {
                SharedLinkInterfacePtr forwardingSupportLink = qgcApp()->toolbox()->linkManager()->mavlinkForwardingSupportLink();
                if (forwardingSupportLink) {
                    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
                    const uint16_t len = mavlink_msg_to_send_buffer(buf, &m_message);
                    forwardingSupportLink->writeBytesThreadSafe(reinterpret_cast<const char*>(buf), len);
                }
            }

            //-----------------------------------------------------------------
            // Log data
            if (!m_logSuspendError && !m_logSuspendReplay && m_tempLogFile->isOpen()) {
                const qint64 timestamp = QDateTime::currentMSecsSinceEpoch() * 1000;
                uint8_t buf[MAVLINK_MAX_PACKET_LEN + sizeof(timestamp)];
                qToBigEndian(timestamp, buf);

                const qsizetype len = mavlink_msg_to_send_buffer(buf + sizeof(timestamp), &m_message) + sizeof(timestamp);
                const QByteArray log_data(reinterpret_cast<const char*>(buf), len);
                if (m_tempLogFile->write(log_data) != len) {
                    const QString message = QStringLiteral("MAVLink Logging failed. Could not write to file %1, logging disabled.").arg(m_tempLogFile->fileName())
                    emit protocolStatusMessage(getName(), message);
                    _stopLogging();
                    m_logSuspendError = true;
                }

                if (!m_vehicleWasArmed && (m_message.msgid == MAVLINK_MSG_ID_HEARTBEAT)) {
                    mavlink_heartbeat_t state;
                    mavlink_msg_heartbeat_decode(&m_message, &state);
                    if (state.base_mode & MAV_MODE_FLAG_DECODE_POSITION_SAFETY) {
                        m_vehicleWasArmed = true;
                    }
                }
            }

            switch (m_message.msgid) {
            case MAVLINK_MSG_ID_HEARTBEAT:
                _startLogging();
                mavlink_heartbeat_t heartbeat;
                mavlink_msg_heartbeat_decode(&m_message, &heartbeat);
                emit vehicleHeartbeatInfo(link, m_message.sysid, m_message.compid, static_cast<MAV_AUTOPILOT>(heartbeat.autopilot), static_cast<MAV_TYPE>(heartbeat.type));
                break;
            case MAVLINK_MSG_ID_HIGH_LATENCY:
                _startLogging();
                mavlink_high_latency_t highLatency;
                mavlink_msg_high_latency_decode(&m_message, &highLatency);
                // HIGH_LATENCY does not provide autopilot or type information, generic is our safest bet
                emit vehicleHeartbeatInfo(link, m_message.sysid, m_message.compid, MAV_AUTOPILOT_GENERIC, MAV_TYPE_GENERIC);
                break;
            case MAVLINK_MSG_ID_HIGH_LATENCY2:
                _startLogging();
                mavlink_high_latency2_t highLatency2;
                mavlink_msg_high_latency2_decode(&m_message, &highLatency2);
                emit vehicleHeartbeatInfo(link, m_message.sysid, m_message.compid, static_cast<MAV_AUTOPILOT>(highLatency2.autopilot), static_cast<MAV_TYPE>(highLatency2.type));
                break;
            default:
                break;
            }

            //-----------------------------------------------------------------
            // Update MAVLink status on every 32th packet
            if ((m_totalReceiveCounter[mavlinkChannel] % 32) == 0) {
                emit mavlinkMessageStatus(m_message.sysid, totalSent, m_totalReceiveCounter[mavlinkChannel], m_totalLossCounter[mavlinkChannel], receiveLossPercent);
            }

            emit messageReceived(link, m_message);

            if (linkPtr.use_count() == 1) {
                break;
            }

            (void) memset(&m_status, 0, sizeof(m_status));
            (void) memset(&m_message, 0, sizeof(m_message));
        }
    }
}

void MAVLinkProtocol::enableVersionCheck(bool enabled)
{
    if (enabled != m_enableVersionCheck) {
        m_enableVersionCheck = enabled;
        emit versionCheckChanged(enabled);
    }
}

void MAVLinkProtocol::_vehicleCountChanged()
{
    if (qgcApp()->toolbox()->multiVehicleManager()->vehicles()->count() == 0) {
        _stopLogging();
        // m_radioVersionMismatchCount = 0;
    }
}

bool MAVLinkProtocol::_closeLogFile()
{
    if (!m_tempLogFile->isOpen()) {
        return false;
    }

    if (m_tempLogFile->size() == 0) {
        m_tempLogFile->remove();
        return false;
    } else {
        m_tempLogFile->flush();
        m_tempLogFile->close();
        return true;
    }
}

void MAVLinkProtocol::_startLogging()
{
    if (qgcApp()->runningUnitTests()) {
        return;
    }

    AppSettings* const appSettings = qgcApp()->toolbox()->settingsManager()->appSettings();
    if (appSettings->disableAllPersistence()->rawValue().toBool()) {
        return;
    }

#ifdef __mobile__
    if (!appSettings->telemetrySave()->rawValue().toBool()) {
        return;
    }
#endif

    if (m_tempLogFile->isOpen()) {
        return;
    }

    if (m_logSuspendReplay) {
        return;
    }

    if (!m_tempLogFile->open()) {
        const QString message = QStringLiteral("Opening Flight Data file for writing failed. Unable to write to %1. Please choose a different file location.").arg(m_tempLogFile->fileName());
        emit protocolStatusMessage(getName(), message);
        _closeLogFile();
        m_logSuspendError = true;
        return;
    }

    qCDebug(MAVLinkProtocolLog) << "Temp log" << m_tempLogFile->fileName();
    emit checkTelemetrySavePath();

    m_logSuspendError = false;
}

void MAVLinkProtocol::_stopLogging()
{
    if (m_tempLogFile->isOpen()) {
        if (_closeLogFile()) {
            AppSettings* const appSettings = qgcApp()->toolbox()->settingsManager()->appSettings();
            if ((m_vehicleWasArmed || appSettings->telemetrySaveNotArmed()->rawValue().toBool()) &&
                appSettings->telemetrySave()->rawValue().toBool() &&
                !appSettings->disableAllPersistence()->rawValue().toBool()) {
                emit saveTelemetryLog(m_tempLogFile->fileName());
            } else {
                (void) QFile::remove(m_tempLogFile->fileName());
            }
        }
    }

    m_vehicleWasArmed = false;
}

void MAVLinkProtocol::checkForLostLogFiles()
{
    static const QDir tempDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    static const QString filter(QString("*.%1").arg(s_logFileExtension));
    const QFileInfoList fileInfoList = tempDir.entryInfoList(QStringList(filter), QDir::Files);
    qCDebug(MAVLinkProtocolLog) << "Orphaned log file count" << fileInfoList.count();

    for(const QFileInfo &fileInfo: fileInfoList) {
        qCDebug(MAVLinkProtocolLog) << "Orphaned log file" << fileInfo.filePath();
        if (fileInfo.size() == 0) {
            (void) QFile::remove(fileInfo.filePath());
            continue;
        }
        emit saveTelemetryLog(fileInfo.filePath());
    }
}

void MAVLinkProtocol::deleteTempLogFiles()
{
    static const QDir tempDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    static const QString filter(QString("*.%1").arg(s_logFileExtension));
    const QFileInfoList fileInfoList = tempDir.entryInfoList(QStringList(filter), QDir::Files);
    qCDebug(MAVLinkProtocolLog) << "Temp log file count" << fileInfoList.count();

    for (const QFileInfo &fileInfo: fileInfoList) {
        qCDebug(MAVLinkProtocolLog) << "Temp log file" << fileInfo.filePath();
        (void) QFile::remove(fileInfo.filePath());
    }
}
