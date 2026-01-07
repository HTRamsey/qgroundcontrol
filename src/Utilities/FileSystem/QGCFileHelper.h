#pragma once

#include <QtCore/QString>

/// Generic file system helper utilities
namespace QGCFileHelper {

// ============================================================================
// Buffer Configuration
// ============================================================================

/// Minimum buffer size for I/O operations
constexpr size_t kBufferSizeMin = 16384;     // 16KB

/// Maximum buffer size for I/O operations
constexpr size_t kBufferSizeMax = 131072;    // 128KB

/// Default buffer size when detection unavailable
constexpr size_t kBufferSizeDefault = 65536; // 64KB

/// Get optimal buffer size for I/O operations (platform-adaptive)
/// Uses filesystem block size when available, with reasonable fallback
/// @param path Optional path to query filesystem block size (empty = use root)
/// @return Buffer size in bytes (16KB-128KB range), cached after first call
size_t optimalBufferSize(const QString &path = QString());

// ============================================================================
// Path Utilities
// ============================================================================

/// Check if path exists (handles Qt resources which always "exist" if valid)
/// @param path File path or Qt resource path (:/...)
/// @return true if path exists
bool exists(const QString &path);

/// Ensure directory exists, creating it if necessary
/// @param path Directory path to ensure exists
/// @return true if directory exists or was created successfully
bool ensureDirectoryExists(const QString &path);

/// Ensure parent directory exists for a file path
/// @param filePath Path to a file (parent directory will be created if needed)
/// @return true if parent directory exists or was created successfully
bool ensureParentExists(const QString &filePath);

// ============================================================================
// Safe File Operations
// ============================================================================

/// Write data to file atomically (prevents corruption on crash/power loss)
/// Uses QSaveFile internally: writes to temp file, then atomically renames.
/// @param filePath Target file path
/// @param data Data to write
/// @return true on success, false on any error (target unchanged on failure)
bool atomicWrite(const QString &filePath, const QByteArray &data);

// ============================================================================
// Disk Space Utilities
// ============================================================================

/// Check if there's sufficient disk space for an operation
/// @param path Target directory or file path
/// @param requiredBytes Bytes needed for the operation
/// @param margin Extra margin multiplier (default 1.1 = 10% safety margin)
/// @return true if sufficient space available, false otherwise
bool hasSufficientDiskSpace(const QString &path, qint64 requiredBytes, double margin = 1.1);

/// Get available disk space at a path
/// @param path Directory or file path to query
/// @return Available bytes, or -1 if unable to determine
qint64 availableDiskSpace(const QString &path);

} // namespace QGCFileHelper
