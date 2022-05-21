#include "RpcServer.h"

#include <climits>
#include <exception>
#include <functional>

#include "json.hpp"
#include "srpc/common/Exception.h"
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
  } catch (const std::exception& e) {
    LOG_ERROR << e.what();
    send_error(conn, {1, e.what(), nullptr}, INT_MIN);
  } catch (...) {
    LOG_FATAL << "unknown error";
    throw;
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
  while (buffer->readable_bytes()) {
    const char* end = buffer->find_EOL();
    LOG_INFO << buffer->readable_bytes();
    if (end == nullptr) {
      buffer->retrieve(buffer->readable_bytes());
      LOG_WARN << "message don't have \\n";
      throw std::logic_error("message don't have \\n");
    }
    if (end == buffer->peek()) {
      buffer->retrieve(buffer->readable_bytes());
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
      send_error(conn, {1, e.what(), nullptr}, INT_MIN);
    } catch (...) {
      LOG_FATAL << "unknown error";
      throw;
    }

    if (json_data.is_object()) {
      try {
        handle_single_request(json_data.get_object(), conn);
      } catch (const common::RpcException& e) {
        LOG_ERROR << e.message();
        send_error(conn, e.get_error_object(), json_data["id"].get_int());
      } catch (const std::exception& e) {
        LOG_ERROR << e.what();
        send_error(conn, {1, e.what(), nullptr}, INT_MIN);
      } catch (...) {
        LOG_FATAL << "unknown error";
        throw;
      }
    } else {
      try {
        handle_multi_request(json_data.get_array(), conn);
      } catch (const common::RpcException& e) {
        LOG_ERROR << e.message();
        send_error(conn, e.get_error_object(), json_data["id"].get_int());
      } catch (const std::exception& e) {
        LOG_ERROR << e.what();
        send_error(conn, {1, e.what(), nullptr}, INT_MIN);
      } catch (...) {
        LOG_FATAL << "unknown error";
        throw;
      }
    }
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
    throw common::RpcException("no notify exists");
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
    throw common::RpcException("no procedure exists");
  }
}

void RpcServer::handle_multi_request(std::vector<s2ujson::JSON_Data>& objects,
                                     const suduo::net::TcpConnectionPtr& conn) {
  std::vector<s2ujson::JSON_Data> responds;
  for (auto& object : objects) {
    common::RequestObject request(object.get_object());
    if (request.is_notifycation()) {
      for (auto& i : _service_list) {
        if (i->notify_exists(request.method())) {
          i->call_notify(request.method(), request);
          break;
        } else if (i == _service_list.back()) {
          responds.emplace_back(
              common::RespondObject(
                  common::RpcException("no notify exists").get_error_object(),
                  request.id())
                  .object());
        }
      }
    } else {
      for (auto& i : _service_list) {
        if (i->procedure_exists(request.method())) {
          i->call_procedure(request, [&request, &conn, &responds](
                                         const s2ujson::JSON_Data& data) {
            responds.emplace_back(
                common::RespondObject(data, request.id()).object());
          });
          break;
        } else if (i == _service_list.back()) {
          responds.emplace_back(
              common::RespondObject(common::RpcException("no procedure exists")
                                        .get_error_object(),
                                    request.id())
                  .object());
        }
      }
    }
  }
  conn->send(s2ujson::JSON_Data(responds).to_string() + '\n');
}

void RpcServer::send_error(const suduo::net::TcpConnectionPtr& conn,
                           const common::ErrorObject& error, int id) {
  common::RespondObject respond(error, id);
  conn->send(respond.to_string() + '\n');
}