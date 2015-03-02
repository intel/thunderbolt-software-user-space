# - Find libnl
#
# This module defines
#  LIBNL_FOUND - whether the libnl library was found
#  LIBNL_LIBRARIES - the libnl library
#  LIBNL_INCLUDE_DIR - the include path of the libnl library

find_library (LIBNL3_LIBRARY nl-3)
find_library (LIBNL1_LIBRARY nl)

set(LIBNL3_LIBRARIES ${LIBNL3_LIBRARY})
set(LIBNL1_LIBRARIES ${LIBNL1_LIBRARY})

find_path (LIBNL3_INCLUDE_DIR
  NAMES
  netlink/version.h
  PATH_SUFFIXES
  libnl3
)

find_path (LIBNL1_INCLUDE_DIR
  NAMES
  netlink/netlink.h
  PATH_SUFFIXES
  libnl
)
