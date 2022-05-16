#ifndef RPC_SERVICE_H
#define RPC_SERVICE_H
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "json.hpp"
#include "srpc/common/FunctionTraits.h"
#include "srpc/common/RequestObject.h"

namespace srpc {
namespace server {
class RpcService {
 public:
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

  void call_procedure(const std::string& name, common::RequestObject& request);
  void call_notify(const std::string& name, common::RequestObject& request);

 private:
  using Functor = std::function<void(common::RequestObject&)>;

  static void send_respone(int id, s2ujson::JSON_Data data);

  std::unordered_map<std::string, Functor> _procedure_map;
  std::unordered_map<std::string, Functor> _notify_map;
};
}  // namespace server
}  // namespace srpc

#endif