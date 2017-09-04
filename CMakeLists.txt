cmake_minimum_required(VERSION 3.5)

project(tbt-userspace)

set(VERSION "0.9.1")

include(GNUInstallDirs)

set(UDEV_RULES_DIR "/lib/udev/rules.d" CACHE PATH "Install path for udev rules")
set(UDEV_BIN_DIR   "/lib/udev"         CACHE PATH "Install path for udev-triggered executables")
set(RULES_PREFIX   "60"                CACHE PATH "The numeric prefix for udev rules file")

set(TBT_CXXFLAGS ${CXX_FLAGS} -Wall -Wextra)

add_subdirectory(common)
add_subdirectory(tbtacl)
add_subdirectory(tbtadm)

set(PACKAGE_SUMMARY "User-space components for handling Thunderbolt controller and devices")

set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Description")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PACKAGE_SUMMARY})
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Thunderbolt Software mailing list <thunderbolt-software@lists.01.org>")
set(CPACK_PACKAGE_VENDOR "Intel")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

# RPM specific
set(CPACK_PACKAGE_RELOCATABLE OFF)
set(CPACK_RPM_PACKAGE_REQUIRES "boost-program-options, boost-filesystem")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/lib"
                                                  "/lib/udev"
                                                  "/lib/udev/rules.d"
                                                  "/usr"
                                                  "/usr/share"
                                                  "/usr/share/doc"
                                                  "/usr/bin")

# DEB specific
file(READ "${CPACK_PACKAGE_DESCRIPTION_FILE}" PACKAGE_DESCRIPTION)
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${PACKAGE_DESCRIPTION})

# Main customization points
set(CPACK_PACKAGE_NAME "thunderbolt-user-space")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")

# For DEB
install(FILES ${CPACK_RESOURCE_FILE_LICENSE}
        DESTINATION share/doc/${CPACK_PACKAGE_NAME}
        RENAME copyright)

include(CPack)
