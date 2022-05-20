#include "RpcServer.h"

#include <functional>

#include "json.hpp"
#include "srpc/common/RequestObject.h"
#include "srpc/common/RespondObject.h"
#include "suduo/base/Logger.h"

using RpcServer = srpc::server::RpcServer;

const size_t HighWaterMark = 65536;
const size_t MaxMessageLen = 100 * 1024 * 1024;

RpcServer::RpcServer(suduo::net::EventLoop* loop,
                     const suduo::net::InetAddress& listen_addr)
    : _server(loop, listen_addr, "Rpc") {
  _server.set_connection_callback(
      std::bind(&RpcServer::on_connection, this, std::placeholders::_1));
  _server.set_message_callback(
      std::bind(&RpcServer::on_message, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  // LOG_INFO << _server.ip_port();
  //_server.set_thread_num(0);
}

void RpcServer::on_connection(const suduo::net::TcpConnectionPtr& conn) {
  if (conn->connected()) {
    LOG_DEBUG << "connection " << conn->peer_address().to_Ip_port() << "is up";
    conn->set_high_water_mark_callback(
        std::bind(&RpcServer::on_high_watermark, this, std::placeholders::_1,
                  std::placeholders::_2),
        HighWaterMark);
  } else {
    LOG_DEBUG << "connection " << conn->peer_address().to_Ip_port()
              << "is down";
  }
}

void RpcServer::on_message(const suduo::net::TcpConnectionPtr& conn,
                           suduo::net::Buffer* buffer, suduo::Timestamp when) {
  try {
    handle_message(conn, buffer);
  }
  // TODO catch Exception
  catch (...) {
  }
}

void RpcServer::on_high_watermark(const suduo::net::TcpConnectionPtr& conn,
                                  size_t mark) {
  LOG_DEBUG << "Connection " << conn->peer_address().to_Ip_port()
            << " high watermark " << mark;
  conn->set_write_complete_callback(
      std::bind(&RpcServer::on_write_complete, this, std::placeholders::_1));
  conn->stop_read();
}

void RpcServer::on_write_complete(const suduo::net::TcpConnectionPtr& conn) {
  LOG_DEBUG << "Connection " << conn->peer_address().to_Ip_port()
            << " write complete";
  conn->start_read();
}

void RpcServer::handle_message(const suduo::net::TcpConnectionPtr& conn,
                               suduo::net::Buffer* buffer) {
  const char* end = buffer->find_EOL();
  if (end == nullptr) {
    // TODO handle
  }
  if (end == buffer->peek()) {
    // TODO handle
  }
  int json_len = end - buffer->peek() + 1;
  // LOG_INFO << buffer->to_string();
  std::string json = buffer->retrieve_as_string(json_len);
  s2ujson::JSON_Data json_data;
  LOG_INFO << json;
  try {
    json_data = s2ujson::JSON_parse(json);
  }
  // TODO handle error
  catch (...) {
  }
  if (json_data.is_object()) {
    handle_single_request(json_data.get_object(), conn);
  } else {
    handle_multi_request(json_data.get_array(), conn);
  }
}

void RpcServer::handle_single_request(
    const s2ujson::JSON_Object& object,
    const suduo::net::TcpConnectionPtr& conn) {
  common::RequestObject request(object);
  if (request.is_notifycation()) {
    for (auto& i : _service_list) {
      if (i->notify_exists(request.method())) {
        i->call_notify(request.method(), request);
        return;
      }
    }
  } else {
    for (auto& i : _service_list) {
      if (i->procedure_exists(request.method())) {
        i->call_procedure(request,
                          [&request, &conn](const s2ujson::JSON_Data& data) {
                            common::RespondObject respond(data, request.id());
                            conn->send(respond.to_string() + '\n');
                          });
        return;
      }
    }
  }
}

void RpcServer::handle_multi_request(std::vector<s2ujson::JSON_Data>& objects,
                                     const suduo::net::TcpConnectionPtr& conn) {
  std::vector<s2ujson::JSON_Data> responds(objects.size());
  for (auto& object : objects) {
    common::RequestObject request(object.get_object());
    if (request.is_notifycation()) {
      for (auto& i : _service_list) {
        if (i->notify_exists(request.method())) {
          i->call_notify(request.method(), request);
          break;
          ;
        }
        // TODO
      }
    } else {
      for (auto& i : _service_list) {
        if (i->procedure_exists(request.method())) {
          i->call_procedure(request, [&request, &conn, &responds](
                                         const s2ujson::JSON_Data& data) {
            responds.emplace_back(
                common::RespondObject(data, request.id()).object());
          });
        }
        break;
        // TODO
      }
    }
  }
  conn->send(s2ujson::JSON_Data(responds).to_string() + '\n');
}

// void RpcServer::send_respond(srpc::common::RespondObject& respond,
//                              const suduo::net::TcpConnectionPtr& conn) {
//   conn->send(respond.to_string());
// }
