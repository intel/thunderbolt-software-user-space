cmake_minimum_required(VERSION 3.5)

project(tbtadm VERSION 0.1 LANGUAGES CXX)

find_package(Boost REQUIRED COMPONENTS filesystem program_options)

add_executable(${PROJECT_NAME} "main.cpp" "controller.cpp" "file.cpp")

target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Werror>)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
