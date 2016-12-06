# - Find LibDBus-c++-1
#
# This module defines
#  LIBDBUS-C++-1_FOUND - whether the libdbus-c++ library was found
#  LIBDBUS-C++-1_LIBRARIES - the libdbus-c++ library
#  LIBDBUS-C++-1_INCLUDE_DIRS - the include path for the libdbus-c++ library

find_library(LIBDBUS-C++-1_LIBRARY dbus-c++-1)
find_library(LIBDBUS-1_LIBRARY dbus-1)

find_program(DBUSXX_XML2CPP dbusxx-xml2cpp)

set(LIBDBUS-C++-1_LIBRARIES ${LIBDBUS-C++-1_LIBRARY})

find_path(LIBDBUS-C++-1_INCLUDE_DIR
          NAMES dbus-c++/dbus.h
          PATH_SUFFIXES dbus-c++-1)

find_program(GREP grep)
execute_process(
   COMMAND ${GREP} -F -q "virtual IntrospectedInterface *const introspect" ${LIBDBUS-C++-1_INCLUDE_DIR}/dbus-c++/interface.h
   RESULT_VARIABLE GREP_RC
   )

if (${GREP_RC} EQUAL "0")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDBUS_CXX_INTROSPECT_HAS_SPURIOUS_CONST=1")
endif()

set(LIBDBUS-C++-1_INCLUDE_DIRS ${LIBDBUS-C++-1_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibDBus-c++-1 DEFAULT_MSG
                                  LIBDBUS-C++-1_INCLUDE_DIR LIBDBUS-C++-1_LIBRARY)

mark_as_advanced(LIBDBUS-C++-1_INCLUDE_DIR LIBDBUS-C++-1_LIBRARY)

