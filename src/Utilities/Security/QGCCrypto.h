#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(QGCCryptoLog)

/// Cryptographic utilities for hashing, HMAC, and secure random generation.
///
/// Uses Qt's QCryptographicHash and OpenSSL (via Qt) for cryptographic operations.
///
/// Example usage:
/// @code
/// // SHA256 hash
/// QByteArray hash = QGCCrypto::sha256("password");
/// QString hexHash = QGCCrypto::sha256Hex("password");
///
/// // HMAC
/// QByteArray hmac = QGCCrypto::hmacSha256("key", "message");
///
/// // Secure random
/// QByteArray random = QGCCrypto::randomBytes(32);
/// QString token = QGCCrypto::randomHex(16);
/// @endcode
class QGCCrypto : public QObject
{
    Q_OBJECT

public:
    explicit QGCCrypto(QObject *parent = nullptr);
    ~QGCCrypto() override = default;

    // ========================================================================
    // Hashing
    // ========================================================================

    /// SHA-256 hash
    static QByteArray sha256(const QByteArray &data);
    static QByteArray sha256(const QString &text);
    static QString sha256Hex(const QByteArray &data);
    static QString sha256Hex(const QString &text);

    /// SHA-512 hash
    static QByteArray sha512(const QByteArray &data);
    static QByteArray sha512(const QString &text);
    static QString sha512Hex(const QByteArray &data);
    static QString sha512Hex(const QString &text);

    /// MD5 hash (not cryptographically secure, for checksums only)
    static QByteArray md5(const QByteArray &data);
    static QString md5Hex(const QByteArray &data);

    /// Hash file contents
    static QByteArray sha256File(const QString &filePath);
    static QString sha256FileHex(const QString &filePath);

    // ========================================================================
    // HMAC
    // ========================================================================

    /// HMAC-SHA256
    static QByteArray hmacSha256(const QByteArray &key, const QByteArray &message);
    static QString hmacSha256Hex(const QByteArray &key, const QByteArray &message);
    static QString hmacSha256Hex(const QString &key, const QString &message);

    /// HMAC-SHA512
    static QByteArray hmacSha512(const QByteArray &key, const QByteArray &message);
    static QString hmacSha512Hex(const QByteArray &key, const QByteArray &message);

    // ========================================================================
    // Random number generation
    // ========================================================================

    /// Generate cryptographically secure random bytes
    static QByteArray randomBytes(int length);

    /// Generate random hex string
    static QString randomHex(int byteLength);

    /// Generate random alphanumeric string
    static QString randomAlphanumeric(int length);

    /// Generate random integer in range [min, max]
    static int randomInt(int min, int max);

    /// Generate UUID v4
    static QString randomUuid();

    // ========================================================================
    // Base64 encoding
    // ========================================================================

    /// Standard Base64 encoding
    static QString toBase64(const QByteArray &data);
    static QByteArray fromBase64(const QString &encoded);

    /// URL-safe Base64 encoding (no padding, + -> -, / -> _)
    static QString toBase64Url(const QByteArray &data);
    static QByteArray fromBase64Url(const QString &encoded);

    // ========================================================================
    // Password hashing
    // ========================================================================

    /// Hash password with salt using PBKDF2-SHA256
    /// @param password Password to hash
    /// @param salt Salt (or empty for auto-generated)
    /// @param iterations Number of iterations (default 100000)
    /// @return Format: "iterations:salt:hash" all base64 encoded
    static QString hashPassword(const QString &password,
                                 const QByteArray &salt = {},
                                 int iterations = 100000);

    /// Verify password against hash
    static bool verifyPassword(const QString &password, const QString &hash);

    // ========================================================================
    // Utility
    // ========================================================================

    /// Constant-time comparison (prevents timing attacks)
    static bool constantTimeCompare(const QByteArray &a, const QByteArray &b);

    /// Securely zero memory
    static void secureZero(QByteArray &data);

    /// Convert to hex string
    static QString toHex(const QByteArray &data);
    static QByteArray fromHex(const QString &hex);
};
