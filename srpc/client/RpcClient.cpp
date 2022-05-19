#include "RpcClient.h"

#include "srpc/common/RespondObject.h"

using RpcClient = srpc::client::RpcClient;

void RpcClient::on_message(const suduo::net::TcpConnectionPtr& conn,
                           suduo::net::Buffer* buffer, suduo::Timestamp when) {}

void RpcClient::connection_callback(const suduo::net::TcpConnectionPtr& conn) {
  if (conn->connected()) {
    LOG_INFO << "COnnected";
    _conn = conn;
  } else {
    LOG_WARN << "disconnected";
    _conn = nullptr;
  }
}

void RpcClient::send_request(common::RequestObject& request,
                             const RpcClientService::RespondHandler& handle) {
  if (!request.is_notifycation()) {
    _respond_map.emplace(request.id(), handle);
  }
  _conn->send(request.to_string());
}
void RpcClient::handle_respond(const s2ujson::JSON_Object& object,
                               const suduo::net::TcpConnectionPtr& conn) {
  common::RespondObject respond(object);
  auto iter = _respond_map.find(respond.id());
  if (iter == _respond_map.end()) {
    // TODO handle error
  }
  *(iter->second.first) = std::move(respond.result());
  iter->second.second->count_down();
}

void RpcClient::handle_message(const suduo::net::TcpConnectionPtr& conn,
                               suduo::net::Buffer* buffer) {
  const char* end = buffer->find_EOL();
  if (end == nullptr) {
    // TODO handle
  }
  if (end == buffer->peek()) {
    // TODO handle
  }
  int json_len = end - buffer->peek();
  std::string json = buffer->retrieve_as_string(json_len);
  s2ujson::JSON_Data json_data;
  try {
    json_data = s2ujson::JSON_parse(json);
  }
  // TODO handle error
  catch (...) {
  }
  if (json_data.is_object()) {
    handle_respond(json_data.get_object(), conn);
  } else {
    // handle batch
  }
}