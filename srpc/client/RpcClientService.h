#ifndef RPC_CLIENT_SERVICE_H
#define RPC_CLIENT_SERVICE_H
#include <limits.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "json.hpp"
#include "srpc/common/RequestObject.h"
#include "srpc/common/RespondObject.h"
#include "suduo/base/BlockingQueue.h"
#include "suduo/base/CountDownLatch.h"
#include "suduo/base/Timestamp.h"
#include "suduo/base/noncopyable.h"
#include "suduo/net/TcpClient.h"
// TODO call batch;
namespace srpc {
namespace client {
using RespondCallback = std::function<void(s2ujson::JSON_Data&)>;
class Batch {
 public:
  using BatchElement = std::pair<common::RequestObject, RespondCallback>;
  template <typename... Args>
  void add_procedure(const std::string& name, RespondCallback callback,
                     Args... args);

  template <typename... Args>
  void add_notify(const std::string& name, Args... args);

  std::vector<BatchElement>& get_batch() { return _batch; }

 private:
  std::vector<BatchElement> _batch;
};
template <typename... Args>
void Batch::add_procedure(const std::string& name, RespondCallback callback,
                          Args... args) {
  auto call_args = std::make_tuple(args...);
  std::vector<s2ujson::JSON_Data> args_array;
  args_array.reserve(sizeof...(args));
  std::apply(
      [&args_array](auto&&... elems) {
        (args_array.push_back(std::forward<decltype(elems)>(elems)), ...);
      },
      call_args);
  common::RequestObject request(name, args_array, -1);
  _batch.emplace_back(request, std::move(callback));
}

template <typename... Args>
void Batch::add_notify(const std::string& name, Args... args) {
  auto call_args = std::make_tuple(args...);
  std::vector<s2ujson::JSON_Data> args_array;
  args_array.reserve(sizeof...(args));
  std::apply(
      [&args_array](auto&&... elems) {
        (args_array.push_back(std::forward<decltype(elems)>(elems)), ...);
      },
      call_args);
  common::RequestObject request(name, args_array);
  _batch.emplace_back(request, nullptr);
}

class RpcClientService : suduo::noncopyable {
 public:
  using SendRequestCallback =
      std::function<void(common::RequestObject&, RespondCallback)>;
  using SendBatchCallback =
      std::function<void(std::vector<Batch::BatchElement>&)>;

  template <typename... Args>
  void call_procedure(const std::string& name, RespondCallback callback,
                      Args... args);

  template <typename... Args>
  void call_notify(const std::string& name, Args... args);

  void call_batch(Batch& batch) {
    auto batch_list = batch.get_batch();
    for (auto& i : batch_list) {
      if (!i.first.is_notifycation()) {
        _count++;
        i.first.set_id(_count);
      }
    }
    _send_batch_callback(batch_list);
  }

  void set_send_respond_callback(SendRequestCallback callback) {
    _send_request_callback = std::move(callback);
  }

  void set_send_batch_callback(SendBatchCallback callback) {
    _send_batch_callback = std::move(callback);
  }

 private:
  std::atomic_int _count;
  SendRequestCallback _send_request_callback;
  SendBatchCallback _send_batch_callback;
};

template <typename... Args>
void RpcClientService::call_procedure(const std::string& name,
                                      RespondCallback callback, Args... args) {
  _count++;
  auto call_args = std::make_tuple(args...);
  std::vector<s2ujson::JSON_Data> args_array;
  args_array.reserve(sizeof...(args));
  std::apply(
      [&args_array](auto&&... elems) {
        (args_array.push_back(std::forward<decltype(elems)>(elems)), ...);
      },
      call_args);
  common::RequestObject request(name, args_array, _count);
  if (_count == INT_MAX) _count = 0;
  _send_request_callback(request, std::move(callback));
}

template <typename... Args>
void RpcClientService::call_notify(const std::string& name, Args... args) {
  auto call_args = std::make_tuple(args...);
  std::vector<s2ujson::JSON_Data> args_array;
  args_array.reserve(sizeof...(args));
  std::apply(
      [&args_array](auto&&... elems) {
        (args_array.push_back(std::forward<decltype(elems)>(elems)), ...);
      },
      call_args);
  common::RequestObject request(name, args_array);
  _send_request_callback(request, nullptr);
}
}  // namespace client
}  // namespace srpc

#endif