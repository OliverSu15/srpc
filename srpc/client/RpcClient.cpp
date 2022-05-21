#include "RpcClient.h"

#include "json.hpp"
#include "srpc/client/RpcClientService.h"
#include "srpc/common/RespondObject.h"

using RpcClient = srpc::client::RpcClient;

void RpcClient::on_message(const suduo::net::TcpConnectionPtr& conn,
                           suduo::net::Buffer* buffer, suduo::Timestamp when) {
  handle_message(conn, buffer);
}

void RpcClient::connection_callback(const suduo::net::TcpConnectionPtr& conn) {
  if (conn->connected()) {
    LOG_INFO << "Connected";
    _conn = conn;
  } else {
    LOG_WARN << "disconnected";
    _conn = nullptr;
  }
}

void RpcClient::send_request(common::RequestObject& request,
                             RespondCallback callback) {
  if (!request.is_notifycation()) {
    _respond_map.emplace(request.id(), std::move(callback));
  }
  _conn->send(request.to_string() + '\n');
}
void RpcClient::send_batch(std::vector<Batch::BatchElement>& batch) {
  s2ujson::JSON_Data requests(std::vector<s2ujson::JSON_Data>(batch.size()));
  for (int i = 0; i < batch.size(); i++) {
    auto& element = batch[i];
    if (!element.first.is_notifycation()) {
      _respond_map.emplace(element.first.id(), std::move(element.second));
    }
    requests.get_array()[i] = element.first.object();
  }
  _conn->send(requests.to_string() + '\n');
}

void RpcClient::handle_respond(const s2ujson::JSON_Object& object) {
  common::RespondObject respond(object);
  if (respond.is_error()) {
    LOG_ERROR << respond.error_code() << " " << respond.error_message();
  } else {
    auto iter = _respond_map.find(respond.id());
    if (iter == _respond_map.end()) {
      LOG_ERROR << "no request find";
    }
    iter->second(respond.result());
    _respond_map.erase(iter);
  }
}
void RpcClient::handle_batch(std::vector<s2ujson::JSON_Data>& array) {
  for (auto& i : array) {
    handle_respond(i.get_object());
  }
}

void RpcClient::handle_message(const suduo::net::TcpConnectionPtr& conn,
                               suduo::net::Buffer* buffer) {
  const char* end = buffer->find_EOL();
  if (end == nullptr) {
    LOG_WARN << "message don't have \\n";
    throw std::logic_error("message don't have \\n");
  }
  if (end == buffer->peek()) {
    LOG_WARN << "message have only one char";
    throw std::logic_error("message have only one char");
  }
  int json_len = end - buffer->peek() + 1;
  std::string json = buffer->retrieve_as_string(json_len);
  s2ujson::JSON_Data json_data;
  try {
    json_data = s2ujson::JSON_parse(json);
  } catch (const std::exception& e) {
    LOG_ERROR << e.what();
  } catch (...) {
    LOG_FATAL << "unknown error";
    throw;
  }
  if (json_data.is_object()) {
    handle_respond(json_data.get_object());
  } else {
    handle_batch(json_data.get_array());
    // handle batch
  }
}