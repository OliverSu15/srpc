#include "RpcClientService.h"

#include <limits.h>

#include <memory>
#include <tuple>

#include "json.hpp"
#include "srpc/common/RequestObject.h"
#include "suduo/base/CountDownLatch.h"

using RpcClientService = srpc::client::RpcClientService;

template <typename... Args>
s2ujson::JSON_Data RpcClientService::call_procedure(const std::string& name,
                                                    Args... args) {
  _count++;
  suduo::CountDownLatch latch(1);
  auto call_args = std::make_tuple(args...);
  std::vector<s2ujson::JSON_Data> args_array;
  for (auto& i : call_args) {
    args_array.emplace_back(i);
  }
  common::RequestObject request(name, args_array, _count);
  if (_count == INT_MAX) _count = 0;
  std::shared_ptr<s2ujson::JSON_Data> result(new s2ujson::JSON_Data);
  _send_request_callback(request, {result, &latch});
  //_queue.push({request, {result, &latch}});
  latch.wait();
  return {*result};
}

template <typename... Args>
void RpcClientService::call_notify(const std::string& name, Args... args) {
  auto call_args = std::make_tuple(args...);
  std::vector<s2ujson::JSON_Data> args_array;
  for (auto& i : call_args) {
    args_array.emplace_back(i);
  }
  common::RequestObject request(name, args_array);
  _send_request_callback(request, {nullptr, nullptr});
  //_queue.push({request, {nullptr, nullptr}});
}