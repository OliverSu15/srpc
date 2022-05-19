#ifndef RPC_CLIENT_SERVICE_H
#define RPC_CLIENT_SERVICE_H
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "json.hpp"
#include "srpc/common/RequestObject.h"
#include "suduo/base/BlockingQueue.h"
#include "suduo/base/CountDownLatch.h"
#include "suduo/base/Timestamp.h"
#include "suduo/net/TcpClient.h"
// TODO call batch;
namespace srpc {
namespace client {
class RpcClientService {
 public:
  using DataPtr = std::shared_ptr<s2ujson::JSON_Data>;
  using RespondHandler = std::pair<DataPtr, suduo::CountDownLatch*>;
  // using RequestQueue = suduo::BlockingQueue<
  //     std::pair<const common::RequestObject&, RespondHandler>>;
  using SendRequestCallback =
      std::function<void(common::RequestObject&, const RespondHandler&)>;

  template <typename... Args>
  s2ujson::JSON_Data call_procedure(const std::string& name, Args... args);

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
}  // namespace client
}  // namespace srpc

#endif