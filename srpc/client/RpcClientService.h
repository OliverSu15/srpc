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
#include "suduo/net/TcpClient.h"
// TODO call batch;
namespace srpc {
namespace client {
class RpcClientService {
 public:
  using RespondCallback = std::function<void(s2ujson::JSON_Data&)>;
  // using DataPtr = std::shared_ptr<s2ujson::JSON_Data>;
  // using RespondHandler = std::pair<DataPtr, suduo::CountDownLatch*>;
  // using RequestQueue = suduo::BlockingQueue<
  //     std::pair<const common::RequestObject&, RespondHandler>>;
  using SendRequestCallback =
      std::function<void(common::RequestObject&, RespondCallback)>;

  template <typename... Args>
  void call_procedure(const std::string& name, RespondCallback callback,
                      Args... args);

  template <typename... Args>
  void call_notify(const std::string& name, Args... args);

  void set_send_respond_callback(SendRequestCallback callback) {
    _send_request_callback = std::move(callback);
  }

  // const RequestQueue& get_queue() { return _queue; }

 private:
  std::atomic_int _count;
  // static RequestQueue _queue;
  SendRequestCallback _send_request_callback;
};

template <typename... Args>
void RpcClientService::call_procedure(const std::string& name,
                                      RespondCallback callback, Args... args) {
  _count++;
  suduo::CountDownLatch latch(1);
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
  //_queue.push({request, {result, &latch}});
  // latch.wait();
  // return 1;
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
  // TODO
  _send_request_callback(request, nullptr);
  //_queue.push({request, {nullptr, nullptr}});
}
}  // namespace client
}  // namespace srpc

#endif