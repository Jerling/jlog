add_executable(jlogd jlogd.cpp)
target_link_libraries(jlogd jlog)

add_executable(client client.cpp)
target_link_libraries(client jlog)
