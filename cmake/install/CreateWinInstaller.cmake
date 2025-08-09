message(STATUS "QGC: Creating Windows NSIS Installer")

foreach(p IN ITEMS
    QGC_WINDOWS_ICON_PATH
    QGC_WINDOWS_INSTALL_HEADER_PATH
    QGC_WINDOWS_INSTALLER_SCRIPT
    QGC_WINDOWS_OUT
    CMAKE_INSTALL_PREFIX)
    if(NOT DEFINED ${p})
        message(FATAL_ERROR "QGC: Missing required var: ${p}")
    endif()
endforeach()

file(TO_CMAKE_PATH "${QGC_WINDOWS_ICON_PATH}" QGC_INSTALLER_ICON)
file(TO_CMAKE_PATH "${QGC_WINDOWS_INSTALL_HEADER_PATH}" QGC_INSTALLER_HEADER_BITMAP)
file(TO_CMAKE_PATH "${QGC_WINDOWS_INSTALLER_SCRIPT}" QGC_NSIS_INSTALLER_SCRIPT)
file(TO_CMAKE_PATH "${QGC_WINDOWS_OUT}" QGC_INSTALLER_OUT)
file(TO_CMAKE_PATH "${CMAKE_INSTALL_PREFIX}" QGC_PAYLOAD_DIR)

set(_pf86 "ProgramFiles(x86)")
set(_PF86 "PROGRAMFILES(x86)")
find_program(QGC_NSIS_INSTALLER_CMD makensis
    PATHS "$ENV{Programfiles}" "$ENV{PROGRAMFILES}" "$ENV{${_pf86}}" "$ENV{${_PF86}}" "$ENV{ProgramW6432}" "$ENV{PROGRAMW6432}"
    PATH_SUFFIXES "NSIS"
    DOC "Path to the makensis utility."
    REQUIRED
)

set(_rsp "${CMAKE_CURRENT_BINARY_DIR}/nsis_args.rsp")
file(WRITE "${_rsp}" "")

macro(_nsis_arg line)
    file(APPEND "${_rsp}" "${line}\n")
endmacro()

# Version define if available
set(_APPVER "${CMAKE_PROJECT_VERSION}")

_nsis_arg("/NOCD")
_nsis_arg("/INPUTCHARSET UTF8")
_nsis_arg("/V4")

_nsis_arg("/DAPPNAME=\"${CMAKE_PROJECT_NAME}\"")
_nsis_arg("/DEXENAME=\"${CMAKE_PROJECT_NAME}\"")
_nsis_arg("/DORGNAME=\"${QGC_ORG_NAME}\"")
_nsis_arg("/DDESTDIR=\"${QGC_PAYLOAD_DIR}\"")

if(EXISTS "${QGC_INSTALLER_ICON}")
    _nsis_arg("/DINSTALLER_ICON=\"${QGC_INSTALLER_ICON}\"")
endif()

if(EXISTS "${QGC_INSTALLER_HEADER_BITMAP}")
    _nsis_arg("/DHEADER_BITMAP=\"${QGC_INSTALLER_HEADER_BITMAP}\"")
endif()

if(_APPVER)
    _nsis_arg("/DAPPVERSION=\"${_APPVER}\"")
endif()

_nsis_arg("/X\"OutFile \\\"${QGC_INSTALLER_OUT}\\\"\"")
_nsis_arg("\"${QGC_NSIS_INSTALLER_SCRIPT}\"")

execute_process(
    COMMAND "${QGC_NSIS_INSTALLER_CMD}" "@${_rsp}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND_ECHO STDOUT
    COMMAND_ERROR_IS_FATAL ANY
)

