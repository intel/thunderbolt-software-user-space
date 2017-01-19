cmake_minimum_required(VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

set(CMAKE_ECLIPSE_VERSION 4.4)

project("libtbtfwu")

set(LIBTBTFWU_VERSION_MAJOR 1)
set(LIBTBTFWU_VERSION_MINOR 0)
set(LIBTBTFWU_VERSION_PATCH 0)

set(ROOT_CODE_DIR ${CMAKE_SOURCE_DIR}/../..)

list(APPEND CMAKE_MODULE_PATH ${ROOT_CODE_DIR}/cmake/Modules)
list(APPEND CMAKE_MODULE_PATH ${ROOT_CODE_DIR}/cmake)

#include modules
find_package(LibDBus-c++-1 REQUIRED)

message(STATUS "Finding dbus")

if (DBUS_FOUND)
    message(STATUS "DBUS_SERVICES_INSTALL_DIR: ${DBUS_SERVICES_INSTALL_DIR}")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Werror")
    set(cpp11_flags "-std=c++11")
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
      set(visibilitymap -Wl,--version-script=${CMAKE_SOURCE_DIR}/visibility.map)
    else()
      set(visibilitymap "")
    endif()
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
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${visibilitymap}"
        CACHE STRING "Flags used by the linker during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings} ${disabled_warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()


#-------------------------= main application =--------------------------

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${ROOT_CODE_DIR}/shared
                    ${LIBDBUS-C++-1_INCLUDE_DIRS})

#add here all subdirectories that need to be compiled
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src LIB_SRC_FILES)

add_library(tbtfwu SHARED ${LIB_SRC_FILES})
add_library(tbt_static STATIC ${LIB_SRC_FILES})
set_target_properties(tbt_static PROPERTIES OUTPUT_NAME tbtfwu)

target_link_libraries(tbtfwu ${LIBDBUS-C++-1_LIBRARIES})

set_target_properties(tbtfwu
    PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/visibility.map)
set_target_properties(tbt_static
    PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/visibility.map)

# Create generated proxy headers.
add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dbus_controller_proxy.h MAIN_DEPENDENCY ${ROOT_CODE_DIR}/dbus_api/dbus_controller_api.xml
    COMMAND dbusxx-xml2cpp ${ROOT_CODE_DIR}/dbus_api/dbus_controller_api.xml --proxy=${CMAKE_CURRENT_BINARY_DIR}/dbus_controller_proxy.h
)
# Create generated proxy headers.
add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dbus_controllers_proxy.h MAIN_DEPENDENCY ${ROOT_CODE_DIR}/dbus_api/dbus_controllers_api.xml
    COMMAND dbusxx-xml2cpp ${ROOT_CODE_DIR}/dbus_api/dbus_controllers_api.xml --proxy=${CMAKE_CURRENT_BINARY_DIR}/dbus_controllers_proxy.h
)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/DBusControllerProxy.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dbus_controller_proxy.h)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/DBusControllersProxy.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dbus_controllers_proxy.h)

#Install section
set(CMAKE_INSTALL_PREFIX "/usr")
get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
if ("${LIB64}" STREQUAL "TRUE")
  set(LIBSUFFIX 64)
else()
  set(LIBSUFFIX "")
endif()
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib${LIBSUFFIX})
endif()
if(NOT DEFINED CMAKE_INSTALL_INCLDIR)
    set(CMAKE_INSTALL_INCLDIR include/tbt)
endif()

find_package(PkgConfig)

install(
    TARGETS tbtfwu tbt_static
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

set (H_FILES)
list (
    APPEND H_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tbt/tbt_fwu.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tbt/tbt_fwu_err.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tbt/tbt_fwu_controller.h
)
install(FILES ${H_FILES} DESTINATION ${CMAKE_INSTALL_INCLDIR})

set(LIBTBTFWU_VERSION ${LIBTBTFWU_VERSION_MAJOR}.${LIBTBTFWU_VERSION_MINOR}.${LIBTBTFWU_VERSION_PATCH})

include(BuildVersion.txt)
set(LIBTBTFWU_FULL_VERSION "${LIBTBTFWU_VERSION}+${BUILD_VERSION}")

set_target_properties(tbtfwu
    PROPERTIES
        VERSION ${LIBTBTFWU_VERSION}
        SOVERSION ${LIBTBTFWU_VERSION_MAJOR}
        PUBLIC_HEADER ${H_FILES}
)

# pkg-config file handling
configure_file("${PROJECT_NAME}.pc.in" "${PROJECT_NAME}.pc" @ONLY)
install(FILES       "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# RPM/DEB package creation
include(rpm-workaround)

set(CPACK_PACKAGE_VERSION ${LIBTBTFWU_FULL_VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "libtbtfwu-dev")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "thunderbolt-software@lists.01.org")
set(CPACK_PACKAGE_VENDOR "Intel")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)

set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")
