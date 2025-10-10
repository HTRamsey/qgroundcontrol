# ==============================================================================
# Git Information Extraction Module
# ==============================================================================
# Extracts version information and repository metadata from Git
# Required for QGroundControl version strings and build identification
# ==============================================================================

find_package(Git QUIET)

if(NOT GIT_FOUND)
    message(WARNING "QGC: Git not found - version information will be limited")
    set(QGC_GIT_BRANCH "unknown")
    set(QGC_GIT_HASH "unknown")
    set(QGC_APP_VERSION_STR "unknown")
    set(QGC_APP_VERSION "0.0.0")
    set(QGC_APP_VERSION_MAJOR "0")
    set(QGC_APP_VERSION_MINOR "0")
    set(QGC_APP_VERSION_PATCH "0")
    return()
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.git")
    message(WARNING "QGC: Not a Git repository - version information will be limited")
    set(QGC_GIT_BRANCH "unknown")
    set(QGC_GIT_HASH "unknown")
    set(QGC_APP_VERSION_STR "unknown")
    set(QGC_APP_VERSION "0.0.0")
    set(QGC_APP_VERSION_MAJOR "0")
    set(QGC_APP_VERSION_MINOR "0")
    set(QGC_APP_VERSION_PATCH "0")
    return()
endif()

message(STATUS "QGC: Extracting version information from Git repository")

# Git submodule management
option(GIT_SUBMODULE "Check submodules during build" OFF)
if(GIT_SUBMODULE)
    message(STATUS "QGC: Updating Git submodules")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE GIT_SUBMODULE_RESULT
        OUTPUT_VARIABLE GIT_SUBMODULE_OUTPUT
        ERROR_VARIABLE GIT_SUBMODULE_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT GIT_SUBMODULE_RESULT EQUAL 0)
        message(WARNING "QGC: Git submodule update failed: ${GIT_SUBMODULE_ERROR}")
    endif()
endif()

include(CMakePrintHelpers)

# ==============================================================================
# Extract Git Branch Name
# ==============================================================================
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE QGC_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_result
)
if(NOT _git_result EQUAL 0)
    set(QGC_GIT_BRANCH "unknown")
endif()

# ==============================================================================
# Extract Git Commit Hash (short)
# ==============================================================================
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE QGC_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_result
)
if(NOT _git_result EQUAL 0)
    set(QGC_GIT_HASH "unknown")
endif()

# ==============================================================================
# Extract Git Version String (from tags)
# ==============================================================================
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE QGC_APP_VERSION_STR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_result
)
if(NOT _git_result EQUAL 0)
    set(QGC_APP_VERSION_STR "0.0.0")
endif()

# ==============================================================================
# Extract Base Version Number
# ==============================================================================
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --abbrev=0 --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE QGC_APP_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_result
)
if(NOT _git_result EQUAL 0)
    set(QGC_APP_VERSION "v0.0.0")
endif()

# ==============================================================================
# Extract Commit Date
# ==============================================================================
if(QGC_STABLE_BUILD)
    set(QGC_APP_DATE_VERSION ${QGC_APP_VERSION})
else()
    # Daily builds use date of last commit
    set(QGC_APP_DATE_VERSION "")
endif()

execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --format=%aI ${QGC_APP_DATE_VERSION}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE QGC_APP_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _git_result
)
if(NOT _git_result EQUAL 0)
    string(TIMESTAMP QGC_APP_DATE "%Y-%m-%dT%H:%M:%S%z")
endif()

# ==============================================================================
# Parse Version Components
# ==============================================================================
# Remove leading 'v' if present
string(FIND "${QGC_APP_VERSION}" "v" QGC_APP_VERSION_VALID)
if(QGC_APP_VERSION_VALID EQUAL 0)
    string(SUBSTRING "${QGC_APP_VERSION}" 1 -1 QGC_APP_VERSION)
endif()

# Extract major.minor.patch components
if(QGC_APP_VERSION MATCHES "([0-9]+)\\.([0-9]+)\\.([0-9]+)")
    set(QGC_APP_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(QGC_APP_VERSION_MINOR ${CMAKE_MATCH_2})
    set(QGC_APP_VERSION_PATCH ${CMAKE_MATCH_3})
else()
    message(WARNING "QGC: Unable to parse version from '${QGC_APP_VERSION}', using 0.0.0")
    set(QGC_APP_VERSION "0.0.0")
    set(QGC_APP_VERSION_MAJOR "0")
    set(QGC_APP_VERSION_MINOR "0")
    set(QGC_APP_VERSION_PATCH "0")
endif()

# ==============================================================================
# Print Version Information
# ==============================================================================
message(STATUS "QGC: Git branch: ${QGC_GIT_BRANCH}")
message(STATUS "QGC: Git commit: ${QGC_GIT_HASH}")
message(STATUS "QGC: Version: ${QGC_APP_VERSION} (${QGC_APP_VERSION_STR})")
message(STATUS "QGC: Build date: ${QGC_APP_DATE}")
