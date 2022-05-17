#ifndef RPC_SERVER_H
#define RPC_SERVER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "json.hpp"
#include "srpc/common/RespondObject.h"
#include "srpc/server/RpcService.h"
#include "suduo/base/Timestamp.h"
#include "suduo/suduo/net/Callbacks.h"
#include "suduo/suduo/net/EventLoop.h"
#include "suduo/suduo/net/InetAddress.h"
#include "suduo/suduo/net/TcpConnection.h"
#include "suduo/suduo/net/TcpServer.h"
namespace srpc {
namespace server {
class RpcServer {
 public:
  RpcServer(suduo::net::EventLoop* loop,
            const suduo::net::InetAddress& listen_addr);

  void add_service(const std::string& name, RpcService* service);  // TODO

  // void send_respond(srpc::common::RespondObject& respond,
  //                   const suduo::net::TcpConnectionPtr& conn);

 private:
  using ServicePtr = std::unique_ptr<RpcService>;
  using ServiceList = std::vector<ServicePtr>;

  void on_connection(const suduo::net::TcpConnectionPtr& conn);
  void on_message(const suduo::net::TcpConnectionPtr& conn,
                  suduo::net::Buffer* buffer, suduo::Timestamp when);
  void on_high_watermark(const suduo::net::TcpConnectionPtr& conn, size_t mark);
  void on_write_complete(const suduo::net::TcpConnectionPtr& conn);

  void handle_message(const suduo::net::TcpConnectionPtr& conn,
                      suduo::net::Buffer* buffer);

  void send_response(const suduo::net::TcpConnectionPtr& conn,
                     const common::RespondObject& response);
  void handle_single_request(const s2ujson::JSON_Object& object,
                             const suduo::net::TcpConnectionPtr& conn);
  void handle_multi_request(std::vector<s2ujson::JSON_Data>& objects,
                            const suduo::net::TcpConnectionPtr& conn);

  suduo::net::TcpServer _server;
  ServiceList _service_list;
};
}  // namespace server
}  // namespace srpc

#endif