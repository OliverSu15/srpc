#ifndef RPC_SERVICE_H
#define RPC_SERVICE_H
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "json.hpp"
#include "srpc/common/FunctionTraits.h"
#include "srpc/common/RequestObject.h"
#include "srpc/common/RespondObject.h"
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
  void bind_notify(const std::string& name, Func func, const common::no_arg&);
  template <typename Func>
  void bind_notify(const std::string& name, Func func, const common::have_arg&);

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

  std::unordered_map<std::string, ProcedureFunctor> _procedure_map;
  std::unordered_map<std::string, NotifyFunctor> _notify_map;
  // SendRespondCallback _send_respond_callback;
};
}  // namespace server
}  // namespace srpc

#endif