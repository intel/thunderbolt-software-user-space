project(tbtadm VERSION 0.1 LANGUAGES CXX)

add_executable(${PROJECT_NAME} "main.cpp" "controller.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE common)

find_package(Boost REQUIRED COMPONENTS program_options)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})

target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<COMPILE_LANGUAGE:CXX>:${TBT_CXXFLAGS}>)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
