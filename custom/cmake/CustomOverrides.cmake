set(QGC_APP_NAME "ZATQGC" CACHE STRING "Application Name" FORCE)
set(QGC_ORG_NAME "Zenith Aerotech" CACHE STRING "Org Name")
set(QGC_ORG_DOMAIN "https://zenithaerotech.com/" CACHE STRING "Domain")
string(TIMESTAMP TODAY "%Y%m%d")
set(QGC_APP_VERSION "${QGC_GIT_HASH}-${TODAY}" CACHE STRING "App Version (Commit Hash - Compile Date)")

set(QGC_DISABLE_APM_PLUGIN_FACTORY ON CACHE BOOL "Disable APM Plugin Factory" FORCE)
set(QGC_DISABLE_PX4_PLUGIN ON CACHE BOOL "Disable PX4 Plugin" FORCE)
set(QGC_DISABLE_PX4_PLUGIN_FACTORY ON CACHE BOOL "Disable PX4 Plugin Factory" FORCE)
