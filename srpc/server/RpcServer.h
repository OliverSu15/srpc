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
class RpcServer : suduo::noncopyable {
 public:
  using ServicePtr = std::shared_ptr<RpcService>;

  RpcServer(suduo::net::EventLoop* loop,
            const suduo::net::InetAddress& listen_addr);

  ServicePtr create_service() {
    _service_list.emplace_back(new RpcService());
    return _service_list.back();
  };
  void start() { _server.start(); }

 private:
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
  void send_error(const suduo::net::TcpConnectionPtr& conn,
                  const common::ErrorObject& error, int id);
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