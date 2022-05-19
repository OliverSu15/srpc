// #include "RpcService.h"

// #include <tuple>
// #include <utility>

// #include "srpc/common/FunctionTraits.h"
// #include "srpc/common/RequestObject.h"
// #include "srpc/common/RespondObject.h"

// using RpcService = srpc::server::RpcService;
// using namespace srpc::common;

// void RpcService::send_respone(int id, s2ujson::JSON_Data data,
//                               const suduo::net::TcpConnectionPtr& conn) {
//   RespondObject respond(data, id);
//   _send_respond_callback(respond, conn);
// }