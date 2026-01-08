# ============================================================================
# Network Utilities
# Common networking helpers, URL utilities, and HTTP request configuration
# ============================================================================

target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        QGCNetworkHelper.cc
        QGCNetworkHelper.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ============================================================================
# http-parser (HTTP status codes, methods, and URL parsing)
# ============================================================================

CPMAddPackage(
    NAME http_parser
    GITHUB_REPOSITORY nodejs/http-parser
    VERSION 2.9.4
    GIT_TAG v2.9.4
    DOWNLOAD_ONLY YES
)

if(http_parser_ADDED)
    # Build http_parser as a static library
    add_library(http_parser_static STATIC ${http_parser_SOURCE_DIR}/http_parser.c)
    target_include_directories(http_parser_static PUBLIC ${http_parser_SOURCE_DIR})
    set_target_properties(http_parser_static PROPERTIES POSITION_INDEPENDENT_CODE ON)

    # Link to our project
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE http_parser_static)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${http_parser_SOURCE_DIR})
    message(STATUS "http-parser support enabled")
endif()
