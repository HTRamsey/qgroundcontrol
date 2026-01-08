# ============================================================================
# FileSystem Utilities
# File operations, downloads, caching, and file system watching
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCArchiveWatcher.cc
        QGCArchiveWatcher.h
        QGCCachedFileDownload.cc
        QGCCachedFileDownload.h
        QGCDownloadQueue.cc
        QGCDownloadQueue.h
        QGCFileDownload.cc
        QGCFileDownload.h
        QGCFileHelper.cc
        QGCFileHelper.h
        QGCFileUpload.cc
        QGCFileUpload.h
        QGCFileWatcher.cc
        QGCFileWatcher.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
