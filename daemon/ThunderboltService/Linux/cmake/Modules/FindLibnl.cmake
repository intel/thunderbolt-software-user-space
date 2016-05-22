# - Find libnl
#
# This module defines
#  LIBNL_FOUND - whether the libnl library was found
#  LIBNL_LIBRARIES - the libnl libraries
#  LIBNL_INCLUDE_DIRS - the include path for the libnl library

find_library(LIBNL3_LIBRARY nl-3)
find_library(LIBNL3_GENL_LIBRARY nl-genl-3)

set(LIBNL3_LIBRARIES ${LIBNL3_GENL_LIBRARY} ${LIBNL3_LIBRARY})

find_path(LIBNL3_INCLUDE_DIR
          NAMES netlink/version.h
          PATH_SUFFIXES libnl3)

set(LIBNL3_INCLUDE_DIRS ${LIBNL3_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libnl DEFAULT_MSG
                                  LIBNL3_INCLUDE_DIR LIBNL3_GENL_LIBRARY LIBNL3_LIBRARY)

mark_as_advanced(LIBNL3_GENL_LIBRARY LIBNL3_LIBRARY LIBNL3_INCLUDE_DIR)

