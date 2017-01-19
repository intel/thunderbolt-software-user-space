cmake_minimum_required(VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

set(CMAKE_ECLIPSE_VERSION 4.4)

project("tbtfwucli")

include(BuildVersion.txt)
set(VERSION "0.0.2+${BUILD_VERSION}")

set(ROOT_CODE_DIR ${CMAKE_SOURCE_DIR}/../..)

#define modules path
list(APPEND CMAKE_MODULE_PATH ${ROOT_CODE_DIR}/cmake/Modules)
list(APPEND CMAKE_MODULE_PATH ${ROOT_CODE_DIR}/cmake)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(TBTLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libtbtfwu/include"
    CACHE PATH "The path to the tbt library headers")

#include modules
find_package(Libnl REQUIRED)
find_package(LibDBus-c++-1 REQUIRED)
find_library(TBT_LIBRARY tbtfwu)

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

include_directories(${TBTLIB_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${LIBDBUS-C++-1_INCLUDE_DIRS}
                    )

add_executable(tbtfwucli tbtfwucli.cpp)

target_link_libraries(tbtfwucli
                      ${TBT_LIBRARY}
                      ${LIBDBUS-C++-1_LIBRARIES}
                      ${LIBDBUS-1_LIBRARY}
                      ${LIBNL3_LIBRARIES}
                      -lstdc++
                      -lpthread)

#Install section
set(CMAKE_INSTALL_PREFIX "/usr")
set(CLI_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")

find_package(PkgConfig)

install(TARGETS tbtfwucli DESTINATION ${CLI_INSTALL_DIR})

include(rpm-workaround)

set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "thunderbolt-fwu-cli")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "thunderbolt-software@lists.01.org")
set(CPACK_PACKAGE_VENDOR "Intel")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)

set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")
