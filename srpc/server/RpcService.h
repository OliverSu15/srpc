#ifndef RPC_SERVICE_H
#define RPC_SERVICE_H
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "json.hpp"
#include "srpc/common/FunctionTraits.h"
#include "srpc/common/RequestObject.h"
#include "srpc/common/RespondObject.h"
#include "suduo/base/Logger.h"
#include "suduo/net/Callbacks.h"

namespace srpc {
namespace server {
class RpcService {
 public:
  using SendRespondCallback = std::function<void(const s2ujson::JSON_Data&)>;

  template <typename Func>
  void bind_procedure(const std::string& name, Func func);
  template <typename Func>
  void bind_procedure(const std::string& name, Func func, const common::no_arg&,
                      const common::return_void&);
  template <typename Func>
  void bind_procedure(const std::string& name, Func func, const common::no_arg&,
                      const common::return_nonvoid&);
  template <typename Func>
  void bind_procedure(const std::string& name, Func func,
                      const common::have_arg&, const common::return_void&);
  template <typename Func>
  void bind_procedure(const std::string& name, Func func,
                      const common::have_arg&, const common::return_nonvoid&);

  template <typename Func>
  void bind_notify(const std::string& name, Func func);
  template <typename Func>
  void bind_notify(const std::string& name, Func func, common::no_arg);
  template <typename Func>
  void bind_notify(const std::string& name, Func func, common::have_arg);

  void call_procedure(common::RequestObject& request,
                      const SendRespondCallback& callback);

  void call_notify(const std::string& name, common::RequestObject& request);

  bool procedure_exists(const std::string& name) {
    return _procedure_map.find(name) != _procedure_map.end();
  }

  bool notify_exists(const std::string& name) {
    return _notify_map.find(name) != _notify_map.end();
  }

  // void set_send_respond_callback(SendRespondCallback callback) {
  //   _send_respond_callback = std::move(callback);
  // }

 private:
  using ProcedureFunctor =
      std::function<void(common::RequestObject&, const SendRespondCallback&)>;
  using NotifyFunctor = std::function<void(common::RequestObject&)>;

  void send_respone(int id, s2ujson::JSON_Data data,
                    const suduo::net::TcpConnectionPtr& conn);

  template <typename Functor, typename... Args, std::size_t... I>
  decltype(auto) call_helper(Functor func, std::tuple<Args...>&& params,
                             std::index_sequence<I...>) {
    return func(std::get<I>(params)...);
  }
  template <typename Functor, typename... Args>
  decltype(auto) call(Functor f, std::tuple<Args...>& args) {
    return call_helper(f, std::forward<std::tuple<Args...>>(args),
                       std::index_sequence_for<Args...>{});
  }

  std::unordered_map<std::string, ProcedureFunctor> _procedure_map;
  std::unordered_map<std::string, NotifyFunctor> _notify_map;
  // SendRespondCallback _send_respond_callback;
};

template <typename Func>
inline void RpcService::bind_procedure(const std::string& name, Func func) {
  using trait = common::function_traits<Func>;
  bind_procedure(name, func, typename common::func_kind_info<Func>::args_kind(),
                 typename common::func_kind_info<Func>::result_kind());
}

template <typename Func>
inline void RpcService::bind_procedure(const std::string& name, Func func,
                                       const common::no_arg&,
                                       const common::return_void&) {
  _procedure_map.insert(
      std::make_pair(name, [func, this](common::RequestObject& request,
                                        const SendRespondCallback& callback) {
        func();
        callback(nullptr);
      }));
}

template <typename Func>
inline void RpcService::bind_procedure(const std::string& name, Func func,
                                       const common::no_arg&,
                                       const common::return_nonvoid&) {
  _procedure_map.insert(std::make_pair(
      name,
      [func, this](common::RequestObject& request,
                   const SendRespondCallback& callback) { callback(func()); }));
}

template <typename Func>
inline void RpcService::bind_procedure(const std::string& name, Func func,
                                       const common::have_arg&,
                                       const common::return_void&) {
  using trait = common::function_traits<Func>;
  using args_type = typename trait::args_type;
  _procedure_map.insert(
      std::make_pair(name, [func, this](common::RequestObject& request,
                                        const SendRespondCallback& callback) {
        constexpr int args_count = std::tuple_size<args_type>::value;
        args_type args;
        request.convert(args);
        call(func, args);
        callback(nullptr);
      }));
}

template <typename Func>
inline void RpcService::bind_procedure(const std::string& name, Func func,
                                       const common::have_arg&,
                                       const common::return_nonvoid&) {
  using trait = common::function_traits<Func>;
  using args_type = typename trait::args_type;
  _procedure_map.insert(
      std::make_pair(name, [func, this](common::RequestObject& request,
                                        const SendRespondCallback& callback) {
        constexpr int args_count = std::tuple_size<args_type>::value;
        args_type args;
        request.convert<args_count - 1>(args);
        callback(call(func, args));
      }));
}

template <typename Func>
inline void RpcService::bind_notify(const std::string& name, Func func) {
  using trait = common::function_traits<Func>;
  bind_notify(name, func, typename trait::args_tag());
}
template <typename Func>
inline void RpcService::bind_notify(const std::string& name, Func func,
                                    common::no_arg) {
  _notify_map.insert(
      std::make_pair(name, [func](common::RequestObject& request) { func(); }));
}
template <typename Func>
inline void RpcService::bind_notify(const std::string& name, Func func,
                                    common::have_arg) {
  using trait = common::function_traits<Func>;
  using args_type = typename trait::args_type;
  _notify_map.insert(
      std::make_pair(name, [func, this](common::RequestObject& request) {
        constexpr int args_count = std::tuple_size<args_type>::value;
        args_type args;
        request.convert<args_count>(args);
        call(func, args);
      }));
}

inline void RpcService::call_procedure(common::RequestObject& request,
                                       const SendRespondCallback& callback) {
  auto iter = _procedure_map.find(request.method());
  if (iter == _procedure_map.end()) {
    // TODO error handle
  }
  iter->second(request, callback);
}

inline void RpcService::call_notify(const std::string& name,
                                    common::RequestObject& request) {
  auto iter = _notify_map.find(name);
  if (iter == _notify_map.end()) {
    // TODO error handle
  }
  iter->second(request);
}
}  // namespace server
}  // namespace srpc

#endif