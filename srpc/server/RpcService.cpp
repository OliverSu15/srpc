#include "RpcService.h"

#include <tuple>
#include <utility>

#include "srpc/common/FunctionTraits.h"
#include "srpc/common/RequestObject.h"
#include "srpc/common/RespondObject.h"

using RpcService = srpc::server::RpcService;
using namespace srpc::common;

template <typename Func>
void RpcService::bind_procedure(const std::string& name, Func func) {
  using trait = common::function_traits<Func>;
  bind_procedure(name, func, typename trait::args_tag(),
                 typename trait::return_tag());
}

template <typename Func>
void RpcService::bind_procedure(const std::string& name, Func func,
                                const no_arg&, const return_void&) {
  _procedure_map.insert(
      std::make_pair(name, [func, this](RequestObject& request,
                                        const SendRespondCallback& callback) {
        func();
        callback(nullptr);
      }));
}

template <typename Func>
void RpcService::bind_procedure(const std::string& name, Func func,
                                const no_arg&, const return_nonvoid&) {
  _procedure_map.insert(std::make_pair(
      name,
      [func, this](RequestObject& request,
                   const SendRespondCallback& callback) { callback(func()); }));
}

template <typename Func>
void RpcService::bind_procedure(const std::string& name, Func func,
                                const have_arg&, const return_void&) {
  using trait = function_traits<Func>;
  using args_type = typename trait::args_type;
  _procedure_map.insert(
      std::make_pair(name, [func, this](RequestObject& request,
                                        const SendRespondCallback& callback) {
        constexpr int args_count = std::tuple_size<args_type>::value;
        args_type args;
        request.convert<args_count>(args);
        std::apply(func, args);
        callback(nullptr);
      }));
}

template <typename Func>
void RpcService::bind_procedure(const std::string& name, Func func,
                                const have_arg&, const return_nonvoid&) {
  using trait = common::function_traits<Func>;
  using args_type = typename trait::args_type;
  _procedure_map.insert(
      std::make_pair(name, [func, this](RequestObject& request,
                                        const SendRespondCallback& callback) {
        constexpr int args_count = std::tuple_size<args_type>::value;
        args_type args;
        request.convert<args_count>(args);
        callback(std::apply(func, args));
      }));
}

template <typename Func>
void RpcService::bind_notify(const std::string& name, Func func) {
  using trait = common::function_traits<Func>;
  bind_notify(name, func, typename trait::args_tag());
}
template <typename Func>
void RpcService::bind_notify(const std::string& name, Func func,
                             const common::no_arg&) {
  _notify_map.insert(
      std::make_pair(name, [func](RequestObject& request) { func(); }));
}
template <typename Func>
void RpcService::bind_notify(const std::string& name, Func func,
                             const common::have_arg&) {
  using trait = function_traits<Func>;
  using args_type = typename trait::args_type;
  _notify_map.insert(std::make_pair(name, [func](RequestObject& request) {
    constexpr int args_count = std::tuple_size<args_type>::value;
    args_type args;
    request.convert<args_count>(args);
    std::apply(func, args);
  }));
}

void RpcService::call_procedure(common::RequestObject& request,
                                const SendRespondCallback& callback) {
  auto iter = _procedure_map.find(request.method());
  if (iter == _procedure_map.end()) {
    // TODO error handle
  }
  iter->second(request, callback);
}

void RpcService::call_notify(const std::string& name,
                             common::RequestObject& request) {
  auto iter = _notify_map.find(name);
  if (iter == _notify_map.end()) {
    // TODO error handle
  }
  iter->second(request);
}

// void RpcService::send_respone(int id, s2ujson::JSON_Data data,
//                               const suduo::net::TcpConnectionPtr& conn) {
//   RespondObject respond(data, id);
//   _send_respond_callback(respond, conn);
// }