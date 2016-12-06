cmake_minimum_required(VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

set(CMAKE_ECLIPSE_VERSION 4.4)

project("thunderboltd")

include(BuildVersion.txt)
set(VERSION ${BUILD_VERSION})

#define modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../cmake/Modules)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../cmake)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(DRIVER_SHARED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../shared"
    CACHE PATH "The path to the shared code with driver")

set(SERVICE_SHARED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Shared"
    CACHE PATH "The path to the shared code with windows")

set(NETLINK_SOCKETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../External/netLink"
    CACHE PATH "The path to the netlink sockets c++ library")

set(BOOST_INLCUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../External/boost"
    CACHE PATH "The path to the boost headers")

#include modules
include(gmock)
find_package(Libnl REQUIRED)
find_package(LibDBus-c++-1 REQUIRED)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Werror")
    set(cpp11_flags "-std=c++11")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
    set(cpp11_flags "")
endif()

#Disabling warnings with false alarms for common idioms.
#Before g++5, there is a warning for an empty brace init for a struct
if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.1)
    set(disabled_warnings "-Wno-missing-field-initializers")
endif()

#Before g++4.9, there is a warning for unused variable even if it has a
#non-trivial d-tor (e.g. ScopeGuard)
if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    set(disabled_warnings "${disabled_warnings} -Wno-unused-variable")
endif()

if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings} ${disabled_warnings} ${cpp11_flags}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${warnings} ${disabled_warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

#-------------------------= main application =--------------------------

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${SERVICE_SHARED_DIR}/include
                    ${SERVICE_SHARED_DIR}/CommonUtils
                    ${DRIVER_SHARED_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${BOOST_INLCUDE_DIR}
                    ${LIBNL3_INCLUDE_DIRS}
                    ${LIBDBUS-C++-1_INCLUDE_DIRS}
                    )

#add here all subdirectories that need to be compiled
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src LIB_SRC_FILES)
aux_source_directory(${SERVICE_SHARED_DIR}/src SERVICE_SHARED_SRC_FILES)
aux_source_directory(${SERVICE_SHARED_DIR}/CommonUtils SERVICE_COMMON_UTILS)
aux_source_directory(${SERVICE_SHARED_DIR}/src/XDomainProperties SERVICE_SHARED_XDOMAIN_SRC_FILES)
aux_source_directory(${NETLINK_SOCKETS_DIR}/src NETLINK_SOCKETS_SRC_FILES)
add_library(lib OBJECT ${LIB_SRC_FILES}
                       ${SERVICE_SHARED_SRC_FILES}
                       ${NETLINK_SOCKETS_SRC_FILES}
                       ${SERVICE_COMMON_UTILS}
                       ${SERVICE_SHARED_XDOMAIN_SRC_FILES})

add_executable(thunderboltd thunderbolt.cpp $<TARGET_OBJECTS:lib>)

target_link_libraries(thunderboltd
                      ${LIBDBUS-C++-1_LIBRARIES}
                      ${LIBNL3_LIBRARIES}
                      -lstdc++
                      -lpthread
                      -lrt)

                   #add_custom_command (
                   #    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dbus/dbus_controller_adapter.h MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/../../dbus_api/dbus_controller_api.xml
                   #    COMMAND ${DBUSXX_XML2CPP} ${CMAKE_CURRENT_SOURCE_DIR}/../../dbus_api/dbus_controller_api.xml --adaptor=${CMAKE_CURRENT_BINARY_DIR}/dbus/dbus_controller_adapter.h
                   #)

                   #set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/DBusController.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dbus/dbus_controller_adapter.h)

                   #add_custom_command (
                   #    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dbus/dbus_controllers_adapter.h MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/../../dbus_api/dbus_controllers_api.xml
                   #    COMMAND ${DBUSXX_XML2CPP} ${CMAKE_CURRENT_SOURCE_DIR}/../../dbus_api/dbus_controllers_api.xml --adaptor=${CMAKE_CURRENT_BINARY_DIR}/dbus/dbus_controllers_adapter.h
                   #)

                   #set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/DBusControllers.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dbus/dbus_controllers_adapter.h)

#Install section
set(CMAKE_INSTALL_PREFIX "/usr")
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib64)
endif()

set(DBUS_CONFIG_DIR "/etc/dbus-1/system.d/")
set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)
set(UDEV_RULES_DIR "/etc/udev/rules.d")
set(DAEMON_INSTALL_DIR "/usr/lib/thunderbolt")

find_package(PkgConfig)

pkg_check_modules(SYSTEMD QUIET "systemd")
pkg_check_modules(DBUS QUIET "dbus-1")
message(STATUS "Finding systemd and dbus")
if (SYSTEMD_FOUND AND "${SYSTEMD_SERVICES_INSTALL_DIR}" STREQUAL "")
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE}
                    --variable=systemdsystemunitdir systemd
                    OUTPUT_VARIABLE SYSTEMD_SERVICES_INSTALL_DIR)
    #remove space, tabs and newline and place output on DBUS_SERVICES_INSTALL_DIR variable
    string(REGEX REPLACE "[ \t\n]+" "" SYSTEMD_SERVICES_INSTALL_DIR
           "${SYSTEMD_SERVICES_INSTALL_DIR}")
elseif (NOT SYSTEMD_FOUND AND SYSTEMD_SERVICES_INSTALL_DIR)
    message(FATAL_ERROR
            "Variable SYSTEMD_SERVICES_INSTALL_DIR is defined, but we can't \
            find systemd using pkg-config")
endif()

if (DBUS_FOUND AND "${DBUS_SERVICES_INSTALL_DIR}" STREQUAL "")
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE}
                    --variable=system_bus_services_dir dbus-1
                    OUTPUT_VARIABLE DBUS_SERVICES_INSTALL_DIR)
    #remove space, tabs and newline and place output on DBUS_SERVICES_INSTALL_DIR variable
    string(REGEX REPLACE "[ \t\n]+" "" DBUS_SERVICES_INSTALL_DIR
           "${DBUS_SERVICES_INSTALL_DIR}")
elseif (NOT DBUS_FOUND AND DBUS_SERVICES_INSTALL_DIR)
    message(FATAL_ERROR
            "Variable DBUS_SERVICES_INSTALL_DIR is defined, but we can't find \
            dbus using pkg-config")
endif()

if (SYSTEMD_FOUND)
    message(STATUS "SYSTEMD_SERVICES_INSTALL_DIR: ${SYSTEMD_SERVICES_INSTALL_DIR}")
endif()

if (DBUS_FOUND)
    message(STATUS "DBUS_SERVICES_INSTALL_DIR: ${DBUS_SERVICES_INSTALL_DIR}")
endif()


if(SYSTEMD_FOUND AND DBUS_FOUND)
    install(FILES ${CONFIG_DIR}/thunderbolt.service DESTINATION ${SYSTEMD_SERVICES_INSTALL_DIR} COMPONENT config)
    install(FILES ${CONFIG_DIR}/com.Intel.Thunderbolt.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR} COMPONENT config)
    install(FILES ${CONFIG_DIR}/thunderbolt.conf DESTINATION ${DBUS_CONFIG_DIR} COMPONENT config)
    install(FILES ${CONFIG_DIR}/10-thunderbolt.rules DESTINATION ${UDEV_RULES_DIR} COMPONENT config)
    install(TARGETS thunderboltd DESTINATION ${DAEMON_INSTALL_DIR} COMPONENT binary)
    install(SCRIPT post_install_script.cmake COMPONENT script)
else()
    message(WARNING
            "Can't install Thunderbolt daemon because systemd or dbus couldn't be found")
endif()

include(rpm-workaround)

set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
set(CPACK_COMPONENTS_ALL config binary)
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "thunderbolt-daemon")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "thunderbolt-software@lists.01.org")
set(CPACK_PACKAGE_VENDOR "Intel")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)

set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")
