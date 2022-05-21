#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <utility>

#include "json.hpp"
#include "srpc/client/RpcClientService.h"
#include "suduo/base/Timestamp.h"
#include "suduo/net/TcpClient.h"
namespace srpc {
namespace client {
class RpcClient : suduo::noncopyable {
 public:
  RpcClient(suduo::net::EventLoop* loop,
            const suduo::net::InetAddress& server_addr)
      : _client(loop, server_addr, "RpcClient") {
    _client.set_message_callback(
        std::bind(&RpcClient::on_message, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
    _client.set_connection_callback(std::bind(&RpcClient::connection_callback,
                                              this, std::placeholders::_1));
  }

  void connect() { _client.connect(); }

  void register_service(RpcClientService& service) {
    service.set_send_respond_callback(std::bind(&RpcClient::send_request, this,
                                                std::placeholders::_1,
                                                std::placeholders::_2));
    service.set_send_batch_callback(
        std::bind(&RpcClient::send_batch, this, std::placeholders::_1));
  }

 private:
  void on_message(const suduo::net::TcpConnectionPtr& conn,
                  suduo::net::Buffer* buffer, suduo::Timestamp when);
  void connection_callback(const suduo::net::TcpConnectionPtr& conn);
  void send_request(common::RequestObject& request, RespondCallback callback);
  void send_batch(std::vector<Batch::BatchElement>& batch);
  void handle_message(const suduo::net::TcpConnectionPtr& conn,
                      suduo::net::Buffer* buffer);
  void handle_respond(const s2ujson::JSON_Object& object);
  void handle_batch(std::vector<s2ujson::JSON_Data>& array);
  suduo::net::TcpConnectionPtr _conn;
  suduo::net::TcpClient _client;
  std::map<int, RespondCallback> _respond_map;
};
}  // namespace client
}  // namespace srpc
#endif