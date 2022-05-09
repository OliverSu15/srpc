file(GLOB base_SRCS "*.cpp")

add_library(srpc_client ${base_SRCS})
target_link_libraries(srpc_client srpc_common)

install(TARGETS srpc_client DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/srpc/client)