add_executable(logger logger_test.cpp)
target_link_libraries(logger jlog)

add_executable(buffer buffer_test.cpp)
target_link_libraries(buffer jlog)
