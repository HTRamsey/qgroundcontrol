message(STATUS "Creating QGC Installer")

include(CMakePrintHelpers)

set(QT_INSTALLER_FRAMEWORK_ROOT ${QT_ROOT_DIR}/../../Tools/QtInstallerFramework)
find_program(QT_INSTALLER_FRAMEWORK ${QT_INSTALLER_FRAMEWORK_ROOT}/*/bin/binarycreator)

set(INSTALLER_ROOT ${CMAKE_SOURCE_DIR}/deploy/installer)
set(DRIVERS_ROOT ${CMAKE_SOURCE_DIR}/deploy/installer/packages/org.mavlink.drivers)
set(QGC_ROOT ${CMAKE_SOURCE_DIR}/deploy/installer/packages/org.mavlink.drivers)

set(INSTALLER_NAME ${CMAKE_PROJECT_NAME}-Installer.exe)

file(COPY ${CMAKE_SOURCE_DIR}/README.md DESTINATION ${INSTALLER_ROOT}/README.md)
file(COPY ${CMAKE_SOURCE_DIR}/.github/COPYING.md DESTINATION ${QGC_ROOT}/meta/license.txt)

file(GLOB_RECURSE FILES_TO_INSTALL RELATIVE ${CMAKE_INSTALL_PREFIX} ${CMAKE_INSTALL_PREFIX}/**)
file(COPY ${FILES_TO_INSTALL} DESTINATION ${QGC_ROOT}/data/)
cmake_print_variables(INSTALLER_ROOT FILES_TO_INSTALL)

execute_process(COMMAND ${QT_INSTALLER_FRAMEWORK} --offline-only -c ${INSTALLER_ROOT}/config/config.xml -p ${INSTALLER_ROOT}/packages ${CMAKE_BINARY_DIR}/${INSTALLER_NAME})

