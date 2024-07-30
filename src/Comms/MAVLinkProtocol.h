/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QLoggingCategory>

#include "MAVLinkLib.h"

class LinkInterface;
class QGCTemporaryFile;

Q_DECLARE_LOGGING_CATEGORY(MAVLinkProtocolLog)

/// MAVLink micro air vehicle protocol reference implementation.
/// MAVLink is a generic communication protocol for micro air vehicles.
/// for more information, please see the official website: https://mavlink.io
class MAVLinkProtocol : public QObject
{
    Q_OBJECT
    // QML_ELEMENT
    // QML_UNCREATABLE("")
    // QML_SINGLETON
    Q_MOC_INCLUDE("LinkInterface.h")

public:
    MAVLinkProtocol(QObject *parent = nullptr);
    ~MAVLinkProtocol();

    static MAVLinkProtocol *instance();

    /// Get the human-friendly name of this protocol
    static QString getName() { return QStringLiteral("MAVLink protocol"); }

    /// Get the system id of this application
    uint8_t getSystemId() const { return m_systemId; }

    /// Get the component id of this application
    static int getComponentId() { return MAV_COMP_ID_MISSIONPLANNER; }

    /// Get protocol version check state
    bool versionCheckEnabled() const { return m_enableVersionCheck; }

    /// Get the protocol version
    static uint8_t getVersion() { return MAVLINK_VERSION; }

    /// Get the currently configured protocol version
    uint8_t getCurrentVersion() const { return m_currentVersion; }

    /// Reset the counters for all metadata for this link.
    void resetMetadataForLink(LinkInterface *link);

    /// Suspend/Restart logging during replay.
    void suspendLogForReplay(bool suspend) { m_logSuspendReplay = suspend; }

    /// Set protocol version
    void setVersion(uint8_t version);

signals:
    /// Heartbeat received on link
    void vehicleHeartbeatInfo(LinkInterface *link, uint8_t vehicleId, uint8_t componentId, MAV_AUTOPILOT vehicleFirmwareType, MAV_TYPE vehicleType);

    /// Message received and directly copied via signal
    void messageReceived(LinkInterface *link, mavlink_message_t message);

    /// Emitted if version check is enabled/disabled
    void versionCheckChanged(bool enabled);

    /// Emitted if a message from the protocol should reach the user
    void protocolStatusMessage(const QString &title, const QString &message);

    /// Emitted if a new system ID was set
    void systemIdChanged(uint8_t systemId);

    void mavlinkMessageStatus(uint8_t sysid, uint64_t totalSent, uint64_t totalReceived, uint64_t totalLoss, float lossPercent);

    /// Emitted if a new radio status packet received
    ///     @param rxerrors receive errors
    ///     @param fixed count of error corrected packets
    ///     @param rssi local signal strength in dBm
    ///     @param remrssi remote signal strength in dBm
    ///     @param txbuf how full the tx buffer is as a percentage
    ///     @param noise background noise level
    ///     @param remnoise remote background noise level
    void radioStatusChanged(LinkInterface *link, uint32_t rxerrors, uint32_t fixed, int rssi, int remrssi, uint32_t txbuf, uint32_t noise, uint32_t remnoise);

    /// Emitted when a temporary telemetry log file is ready for saving
    void saveTelemetryLog(const QString &tempLogfile);

    /// Emitted when a telemetry log is started to save.
    void checkTelemetrySavePath();

public slots:
    /// Receive bytes from a communication interface and constructs a MAVLink packet
    ///     @param link The interface to read from
    void receiveBytes(LinkInterface *link, const QByteArray &data);

    /// Log bytes sent from a communication interface and logs a MAVLink packet.
    /// It can handle multiple links in parallel, as each link has it's own buffer/parsing state machine.
    ///     @param link The interface to read from
    void logSentBytes(LinkInterface *link, const QByteArray &data);

    /// Set the system id of this application
    void setSystemId(uint8_t id) { m_systemId = id; }

    /// Enable/Disable version check
    void enableVersionCheck(bool enabled);

    /// Load protocol settings
    void loadSettings();

    /// Store protocol settings
    void storeSettings();

    /// Deletes any log files which are in the temp directory
    static void deleteTempLogFiles();

    /// Checks the temp directory for log files which may have been left there.
    /// This could happen if QGC crashes without the temp log file being saved.
    /// Give the user an option to save these orphaned files.
    void checkForLostLogFiles();

private slots:
    void _vehicleCountChanged();

protected:
    bool m_enableVersionCheck = false;                          ///< Enable checking of version match of MAV and QGC
    uint8_t m_lastIndex[256][256];                              ///< Store the last received sequence ID for each system/componenet pair
    uint8_t m_firstMessage[256][256];                           ///< First message flag
    uint64_t m_totalReceiveCounter[MAVLINK_COMM_NUM_BUFFERS];   ///< The total number of successfully received messages
    uint64_t m_totalLossCounter[MAVLINK_COMM_NUM_BUFFERS];      ///< Total messages lost during transmission.
    float m_runningLossPercent[MAVLINK_COMM_NUM_BUFFERS];       ///< Loss rate

    mavlink_message_t m_message{};
    mavlink_status_t m_status{};

    uint8_t m_systemId = 255;
    uint16_t m_currentVersion = 1;
    // uint32_t m_radioVersionMismatchCount = 0;

private:
    /// Closes the log file if it is open
    bool _closeLogFile();
    void _startLogging();
    void _stopLogging();

    bool m_logSuspendError = false;  ///< true: Logging suspended due to error
    bool m_logSuspendReplay = false; ///< true: Logging suspended due to replay
    bool m_vehicleWasArmed = false;  ///< true: Vehicle was armed during log sequence

    QGCTemporaryFile *m_tempLogFile = nullptr;

    static constexpr const char *s_tempLogFileTemplate = "FlightDataXXXXXX"; ///< Template for temporary log file
    static constexpr const char *s_logFileExtension = "mavlink";             ///< Extension for log files
};
