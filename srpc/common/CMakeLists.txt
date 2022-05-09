file(GLOB base_SRCS "*.cpp")

add_library(srpc_common ${base_SRCS})
target_link_libraries(srpc_common)

install(TARGETS srpc_common DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/srpc/common)