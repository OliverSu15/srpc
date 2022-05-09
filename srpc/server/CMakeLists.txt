file(GLOB base_SRCS "*.cpp")

add_library(srpc_server ${base_SRCS})
target_link_libraries(srpc_server srpc_common)

install(TARGETS srpc_server DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/srpc/server)