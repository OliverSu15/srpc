add_executable(Add_client client.cpp)
target_link_libraries(Add_client srpc_client)

add_executable(Add_server server.cpp)
target_link_libraries(Add_server srpc_server)