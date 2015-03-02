add_gmock_test(libTest ToDoTest.cpp $<TARGET_OBJECTS:lib>)
target_link_libraries(libTest ${LIBDBUS-C++-1_LIBRARY} ${LIBNL1_LIBRARIES} ${LIBNL3_LIBRARIES} -lstdc++)

