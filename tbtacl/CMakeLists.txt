project(tbtacl-write VERSION 0.1 LANGUAGES CXX)

add_executable(${PROJECT_NAME} "write.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE common)

target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Werror>)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
