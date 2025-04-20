/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "MAVLinkFTP.h"

Q_DECLARE_LOGGING_CATEGORY(FTPManagerLog)

class Vehicle;

class FTPManager : public QObject
{
    Q_OBJECT

    friend class Vehicle;
public:
    FTPManager(Vehicle *vehicle);

	/// Downloads the specified file.
    ///     @param fromCompId Component id of the component to download from. If fromCompId is MAV_COMP_ID_ALL, then MAV_COMP_ID_AUTOPILOT1 is used.
    ///     @param fromURI    File to download from component, fully qualified path. May be in the format "mftp://[;comp=<id>]..." where the component id
    ///                       is specified. If component id is not specified, then the id set via fromCompId is used.
    ///     @param toDir      Local directory to download file to
    ///     @param filename   (optional)
    ///     @param checksize  (optional, default true) If true compare the filesize indicated in the open
    ///                       response with the transmitted filesize. If false the transmission is tftp style
    ///                       and the indicated filesize from MAVFTP fileopen response is ignored.
    ///                       This is used for the APM parameter download where the filesize is wrong due to
    ///                       a dynamic file creation on the vehicle.
    /// @return true: download has started, false: error, no download
    /// Signals downloadComplete, commandProgress
    bool download(uint8_t fromCompId, const QString &fromURI, const QString &toDir, const QString &fileName = QString(""), bool checksize = true);

	/// Get the directory listing of the specified directory.
    ///     @param fromCompId Component id of the component to download from. If fromCompId is MAV_COMP_ID_ALL, then MAV_COMP_ID_AUTOPILOT1 is used.
    ///     @param fromURI    Directory path to list from component. May be in the format "mftp://[;comp=<id>]..." where the component id
    ///                       is specified. If component id is not specified, then the id set via fromCompId is used.
    /// @return true: process has started, false: error
    /// Signals listDirectoryComplete
    bool listDirectory(uint8_t fromCompId, const QString &fromURI);

    /// Cancel the download operation
    /// This will emit downloadComplete() when done, and if there's currently a download in progress
    void cancelDownload();

    static constexpr const char *mavlinkFTPScheme = "mftp";

signals:
    void downloadComplete(const QString &file, const QString &errorMsg);
    void listDirectoryComplete  (const QStringList &dirList, const QString &errorMsg);

    /// Signalled during a lengthy command to show progress
    ///     @param value Amount of progress: 0.0 = none, 1.0 = complete
    void commandProgress(float value);

private slots:
    void _ackOrNakTimeout();

private:
    void _mavlinkMessageReceived(const mavlink_message_t &message);
    void _startStateMachine();
    void _advanceStateMachine();
    void _listDirectoryBegin();
    void _listDirectoryAckOrNak(const MavlinkFTP::Request *ackOrNak);
    void _listDirectoryTimeout();
    void _openFileROBegin();
    void _openFileROAckOrNak(const MavlinkFTP::Request *ackOrNak);
    void _openFileROTimeout();
    void _burstReadFileBegin();
    void _burstReadFileAckOrNak(const MavlinkFTP::Request *ackOrNak);
    void _burstReadFileTimeout();
    void _fillMissingBlocksBegin();
    void _fillMissingBlocksAckOrNak(const MavlinkFTP::Request *ackOrNak);
    void _fillMissingBlocksTimeout();
    void _resetSessionsBegin();
    void _resetSessionsAckOrNak(const MavlinkFTP::Request *ackOrNak);
    void _resetSessionsTimeout();
    QString _errorMsgFromNak(const MavlinkFTP::Request *nak);
    void _sendRequestExpectAck(MavlinkFTP::Request *request);
    void _downloadCompleteNoError() { _downloadComplete(QString()); }
    /// Closes out a download session by writing the file and doing cleanup.
    ///     @param errorMsg Error message, empty if no error
    void _downloadComplete(const QString &errorMsg);
    void _fillRequestDataWithString(MavlinkFTP::Request *request, const QString &str);
    void _fillMissingBlocksWorker(bool firstRequest);
    void _burstReadFileWorker(bool firstRequest);
    void _listDirectoryWorker(bool firstRequest);
    bool _parseURI(uint8_t fromCompId, const QString &uri, QString &parsedURI, uint8_t &compId);
    bool _isListDirectoryStateMachine();
    void _listDirectoryCompleteNoError() { _listDirectoryComplete(QString()); }
    /// Closes out a list directory sequence
    ///     @param errorMsg Error message, empty if no error
    void _listDirectoryComplete(const QString &errorMsg);

    void _terminateSessionBegin();
    void _terminateSessionAckOrNak(const MavlinkFTP::Request *ackOrNak);
    void _terminateSessionTimeout();
    void _terminateComplete();

    typedef void (FTPManager::*StateBeginFn)();
    typedef void (FTPManager::*StateAckNakFn)(const MavlinkFTP::Request *ackOrNak);
    typedef void (FTPManager::*StateTimeoutFn)();

    struct StateFunctions_t {
        StateBeginFn beginFn = nullptr;
        StateAckNakFn ackNakFn = nullptr;
        StateTimeoutFn timeoutFn = nullptr;
    };
    QList<StateFunctions_t> _rgStateMachine;

    struct MissingData_t {
        uint32_t offset = 0;
        uint32_t cBytesMissing = 0;
    };

    struct DownloadState_t {
        bool inProgress() const { return fileSize > 0; }
        void reset()
        {
            sessionId = 0;
            expectedOffset = 0;
            bytesWritten = 0;
            retryCount = 0;
            fileSize = 0;
            fullPathOnVehicle.clear();
            fileName.clear();
            rgMissingData.clear();
            file.close();
        }

        uint8_t sessionId = 0;
        uint32_t expectedOffset = 0;         ///< offset which should be coming next
        uint32_t bytesWritten;
        QList<MissingData_t> rgMissingData;
        QString fullPathOnVehicle;          ///< Fully qualified path to file on vehicle
        QDir toDir;                         ///< Directory to download file to
        QString fileName;                   ///< Filename (no path) for download file
        uint32_t fileSize = 0;              ///< Size of file being downloaded
        QFile file;
        int retryCount = 0;
        bool checksize = false;
    };
    DownloadState_t _downloadState{};

    struct ListDirectoryState_t {
        bool inProgress() const { return rgDirectoryList.count() > 0; }
        void reset()
        {
            sessionId = 0;
            expectedOffset = 0;
            fullPathOnVehicle.clear();
            rgDirectoryList.clear();
            retryCount = 0;
        }

        uint8_t sessionId = 0;
        uint32_t expectedOffset = 0;    ///< offset which should be coming next
        QString fullPathOnVehicle;      ///< Fully qualified path to file on vehicle
        QStringList rgDirectoryList;
        int retryCount = 0;
    };
    ListDirectoryState_t _listDirectoryState{};

    Vehicle *_vehicle = nullptr;
    uint8_t _ftpCompId = MAV_COMP_ID_AUTOPILOT1;
    QTimer _ackOrNakTimeoutTimer;
    int _currentStateMachineIndex = -1;
    uint16_t _expectedIncomingSeqNumber = 0;

    static constexpr int _ackOrNakTimeoutMsecs = 1000;
    static constexpr int _maxRetry = 3;
};

