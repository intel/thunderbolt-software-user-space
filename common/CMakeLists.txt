project(common VERSION 0.1 LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC "file.cpp")

find_package(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})

target_include_directories(${PROJECT_NAME} INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<COMPILE_LANGUAGE:CXX>:${TBT_CXXFLAGS}>)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
