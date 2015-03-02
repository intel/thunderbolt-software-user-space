# - Find LibDBus-c++-1
#
# This module defines
#  LIBDBUS-C++-1_FOUND - whether the libnl library was found
#  LIBDBUS-C++-1_LIBRARIES - the libnl library

find_library(LIBDBUS-C++-1_LIBRARY dbus-c++-1)
set(LIBNL3_LIBRARIES ${LIBNL3_LIBRARY})

find_path (LIBDBUS-C++-1_INCLUDE_DIR
  NAMES
  dbus-c++/dbus.h
  PATH_SUFFIXES
  dbus-c++-1
)
