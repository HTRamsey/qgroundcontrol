#include "QGCCrypto.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtCore/QUuid>
#include <QtCore/QRandomGenerator>
#include <QtCore/QMessageAuthenticationCode>

#include <cstring>

Q_LOGGING_CATEGORY(QGCCryptoLog, "Utilities.QGCCrypto")

QGCCrypto::QGCCrypto(QObject *parent)
    : QObject(parent)
{
}

// ============================================================================
// Hashing
// ============================================================================

QByteArray QGCCrypto::sha256(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256);
}

QByteArray QGCCrypto::sha256(const QString &text)
{
    return sha256(text.toUtf8());
}

QString QGCCrypto::sha256Hex(const QByteArray &data)
{
    return toHex(sha256(data));
}

QString QGCCrypto::sha256Hex(const QString &text)
{
    return sha256Hex(text.toUtf8());
}

QByteArray QGCCrypto::sha512(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Sha512);
}

QByteArray QGCCrypto::sha512(const QString &text)
{
    return sha512(text.toUtf8());
}

QString QGCCrypto::sha512Hex(const QByteArray &data)
{
    return toHex(sha512(data));
}

QString QGCCrypto::sha512Hex(const QString &text)
{
    return sha512Hex(text.toUtf8());
}

QByteArray QGCCrypto::md5(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}

QString QGCCrypto::md5Hex(const QByteArray &data)
{
    return toHex(md5(data));
}

QByteArray QGCCrypto::sha256File(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(QGCCryptoLog) << "Failed to open file for hashing:" << filePath;
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        qCWarning(QGCCryptoLog) << "Failed to hash file:" << filePath;
        return {};
    }

    return hash.result();
}

QString QGCCrypto::sha256FileHex(const QString &filePath)
{
    return toHex(sha256File(filePath));
}

// ============================================================================
// HMAC
// ============================================================================

QByteArray QGCCrypto::hmacSha256(const QByteArray &key, const QByteArray &message)
{
    return QMessageAuthenticationCode::hash(message, key, QCryptographicHash::Sha256);
}

QString QGCCrypto::hmacSha256Hex(const QByteArray &key, const QByteArray &message)
{
    return toHex(hmacSha256(key, message));
}

QString QGCCrypto::hmacSha256Hex(const QString &key, const QString &message)
{
    return hmacSha256Hex(key.toUtf8(), message.toUtf8());
}

QByteArray QGCCrypto::hmacSha512(const QByteArray &key, const QByteArray &message)
{
    return QMessageAuthenticationCode::hash(message, key, QCryptographicHash::Sha512);
}

QString QGCCrypto::hmacSha512Hex(const QByteArray &key, const QByteArray &message)
{
    return toHex(hmacSha512(key, message));
}

// ============================================================================
// Random number generation
// ============================================================================

QByteArray QGCCrypto::randomBytes(int length)
{
    QByteArray result(length, Qt::Uninitialized);
    QRandomGenerator::securelySeeded().fillRange(
        reinterpret_cast<quint32 *>(result.data()),
        (length + sizeof(quint32) - 1) / sizeof(quint32));
    result.truncate(length);
    return result;
}

QString QGCCrypto::randomHex(int byteLength)
{
    return toHex(randomBytes(byteLength));
}

QString QGCCrypto::randomAlphanumeric(int length)
{
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static const int charsetSize = sizeof(charset) - 1;

    QString result;
    result.reserve(length);

    const QByteArray randomData = randomBytes(length);
    for (int i = 0; i < length; ++i) {
        result.append(QLatin1Char(charset[static_cast<unsigned char>(randomData[i]) % charsetSize]));
    }

    return result;
}

int QGCCrypto::randomInt(int min, int max)
{
    if (min >= max) {
        return min;
    }
    return QRandomGenerator::securelySeeded().bounded(min, max + 1);
}

QString QGCCrypto::randomUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

// ============================================================================
// Base64 encoding
// ============================================================================

QString QGCCrypto::toBase64(const QByteArray &data)
{
    return QString::fromLatin1(data.toBase64());
}

QByteArray QGCCrypto::fromBase64(const QString &encoded)
{
    return QByteArray::fromBase64(encoded.toLatin1());
}

QString QGCCrypto::toBase64Url(const QByteArray &data)
{
    QString result = QString::fromLatin1(data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
    return result;
}

QByteArray QGCCrypto::fromBase64Url(const QString &encoded)
{
    return QByteArray::fromBase64(encoded.toLatin1(), QByteArray::Base64UrlEncoding);
}

// ============================================================================
// Password hashing
// ============================================================================

QString QGCCrypto::hashPassword(const QString &password,
                                 const QByteArray &salt,
                                 int iterations)
{
    QByteArray actualSalt = salt.isEmpty() ? randomBytes(16) : salt;

    // PBKDF2 using Qt's built-in implementation
    QByteArray hash = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256,
        password.toUtf8(),
        actualSalt,
        iterations,
        32  // key length
    );

    // Format: iterations:salt:hash (all base64)
    return QStringLiteral("%1:%2:%3")
        .arg(iterations)
        .arg(toBase64(actualSalt))
        .arg(toBase64(hash));
}

bool QGCCrypto::verifyPassword(const QString &password, const QString &hash)
{
    const QStringList parts = hash.split(':');
    if (parts.size() != 3) {
        return false;
    }

    bool ok = false;
    const int iterations = parts[0].toInt(&ok);
    if (!ok || iterations <= 0) {
        return false;
    }

    const QByteArray salt = fromBase64(parts[1]);
    const QByteArray expectedHash = fromBase64(parts[2]);

    const QByteArray actualHash = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256,
        password.toUtf8(),
        salt,
        iterations,
        expectedHash.size()
    );

    return constantTimeCompare(actualHash, expectedHash);
}

// ============================================================================
// Utility
// ============================================================================

bool QGCCrypto::constantTimeCompare(const QByteArray &a, const QByteArray &b)
{
    if (a.size() != b.size()) {
        return false;
    }

    volatile unsigned char result = 0;
    for (int i = 0; i < a.size(); ++i) {
        result |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }

    return result == 0;
}

void QGCCrypto::secureZero(QByteArray &data)
{
    volatile char *p = data.data();
    for (int i = 0; i < data.size(); ++i) {
        p[i] = 0;
    }
}

QString QGCCrypto::toHex(const QByteArray &data)
{
    return QString::fromLatin1(data.toHex());
}

QByteArray QGCCrypto::fromHex(const QString &hex)
{
    return QByteArray::fromHex(hex.toLatin1());
}
