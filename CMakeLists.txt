cmake_minimum_required(VERSION 3.5)

project(tbt-userspace)

set(TBT_CXXFLAGS ${CXX_FLAGS} -Wall -Wextra)

add_subdirectory(common)
add_subdirectory(tbtacl)
add_subdirectory(tbtadm)
