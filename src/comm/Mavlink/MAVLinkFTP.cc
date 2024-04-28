#include "MAVLinkFTP.h"

QString MavlinkFTP::opCodeToString(OpCode_t opCode)
{
    switch (opCode) {
    case kCmdNone:
        return QStringLiteral("None");
    case kCmdTerminateSession:
        return QStringLiteral("Terminate Session");
    case kCmdResetSessions:
        return QStringLiteral("Reset Sessions");
    case kCmdListDirectory:
        return QStringLiteral("List Directory");
    case kCmdOpenFileRO:
        return QStringLiteral("Open File RO");
    case kCmdReadFile:
        return QStringLiteral("Read File");
    case kCmdCreateFile:
        return QStringLiteral("Create File");
    case kCmdWriteFile:
        return QStringLiteral("Write File");
    case kCmdRemoveFile:
        return QStringLiteral("Remove File");
    case kCmdCreateDirectory:
        return QStringLiteral("Create Directory");
    case kCmdRemoveDirectory:
        return QStringLiteral("Remove Directory");
    case kCmdOpenFileWO:
        return QStringLiteral("Open File WO");
    case kCmdTruncateFile:
        return QStringLiteral("Truncate File");
    case kCmdRename:
        return QStringLiteral("Rename");
    case kCmdCalcFileCRC32:
        return QStringLiteral("Calc File CRC32");
    case kCmdBurstReadFile:
        return QStringLiteral("Burst Read File");
    case kRspAck:
        return QStringLiteral("Ack");
    case kRspNak:
        return QStringLiteral("Nak");
    default:
        return QStringLiteral("Unknown OpCode");
    }
}

QString MavlinkFTP::errorCodeToString(ErrorCode_t errorCode)
{
    switch (errorCode) {
    case kErrNone:
        return QStringLiteral("None");
    case kErrFail:
        return QStringLiteral("Fail");
    case kErrFailErrno:
        return QStringLiteral("Fail Errorno");
    case kErrInvalidDataSize:
        return QStringLiteral("Invalid Data Size");
    case kErrInvalidSession:
        return QStringLiteral("Invalid Session");
    case kErrNoSessionsAvailable:
        return QStringLiteral("No Sessions Available");
    case kErrEOF:
        return QStringLiteral("EOF");
    case kErrUnknownCommand:
        return QStringLiteral("Unknown Command");
    case kErrFailFileExists:
        return QStringLiteral("File Already Exists");
    case kErrFailFileProtected:
        return QStringLiteral("File Protected");
    case kErrFailFileNotFound:
        return QStringLiteral("File Not Found");
    default:
        return QStringLiteral("Unknown Error");
    }
}

