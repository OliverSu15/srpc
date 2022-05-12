#ifndef PROCEDURE_H_
#define PROCEDURE_H_
#include <functional>
#include <string>
#include <vector>

#include "json.hpp"
#include "srpc/common/RequestObject.h"
#include "suduo/suduo/base/noncopyable.h"

using RpcReturn = std::function<void(s2ujson::JSON_Data)>;

namespace srpc {
namespace server {

class Param {
  Param(const std::string& param_name, s2ujson::JSON_Type param_type)
      : name(param_name), type(param_type) {}
  std::string name;
  s2ujson::JSON_Type type;
};

using ProcedureCallback =
    std::function<void(common::RequestObject&, const RpcReturn&)>;

using NotifyCallback = std::function<void(common::RequestObject&)>;

template <typename Func>
class Procedure : suduo::noncopyable {
 public:
  template <typename... ParamNameAndTypes>
  explicit Procedure(Func&& func, ParamNameAndTypes&&... param_list)
      : _func(std::forward<Func>(func)) {
    constexpr int n = sizeof...(param_list);
    if constexpr (n > 0) init_param_list(param_list...);
  }

  void invoke(common::RequestObject& request, const RpcReturn& return_callback);
  // notify
  void invoke(common::RequestObject& request);

 private:
  template <typename Name, typename... ParamNameAndTypes>
  void init_param_list(Name param_name, s2ujson::JSON_Type param_type,
                       ParamNameAndTypes&&... next) {
    _param_list.emplace_back(param_name, param_type);
    if constexpr (sizeof...(ParamNameAndTypes) > 0) {
      init_param_list(next...);
    }
  }

  Func _func;
  std::vector<Param> _param_list;
};
}  // namespace server
}  // namespace srpc
#endif