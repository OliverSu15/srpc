#ifndef REQUEST_OBJECT_H
#define REQUEST_OBJECT_H
#include <cstddef>
#include <cstdint>
#include <exception>
#include <json.hpp>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "suduo/base/Logger.h"

namespace srpc {
namespace common {

namespace detail {
struct NormalTag {};
struct EndTag {};
template <std::size_t T>
struct RecursiveTrait {
  using Tag = NormalTag;
};

template <>
struct RecursiveTrait<0> {
  using Tag = EndTag;
};
}  // namespace detail
using JsonData = s2ujson::JSON_Data;
using JsonObject = s2ujson::JSON_Object;
using JsonArray = std::vector<s2ujson::JSON_Data>;
using string = std::string;
static const std::string json_rpc_stirng = "jsonrpc";
static const std::string json_rpc_v2_stirng = "2.0";
static const std::string json_method_stirng = "method";
static const std::string json_params_stirng = "params";
static const std::string json_id_stirng = "id";

class RequestObject {
 public:
  explicit RequestObject(const JsonObject& object) : _object(object) {
    if (!_object.exist(json_rpc_stirng)) {
      // TODO handle error
    }
    if (!_object.exist(json_method_stirng)) {
      // TODO handle error
    }
    _param_type = ByIndex;
    _notifycation = !object.exist(json_id_stirng);
  }
  // ctor for notifycation
  explicit RequestObject(const string& method_name) : RequestObject() {
    _object.add(json_method_stirng, method_name);
  }
  // RequestObject(const string& method_name, const JsonObject& param)
  //     : RequestObject(method_name) {
  //   _object.add(json_params_stirng, param);
  //   _param_type = ByName;
  // }
  RequestObject(const string& method_name, const JsonArray& param)
      : RequestObject(method_name) {
    _object.add(json_params_stirng, param);
    _param_type = ByIndex;
  }

  // ctor for normal request
  RequestObject(const string& method_name, int id)
      : RequestObject(method_name) {
    _object.add(json_id_stirng, id);
    _notifycation = false;
  }
  // RequestObject(const string& method_name, const JsonObject& param, int id)
  //     : RequestObject(method_name, id) {
  //   _object.add(json_params_stirng, param);
  //   _param_type = ByName;
  //   _notifycation = false;
  // }
  RequestObject(const string& method_name, const JsonArray& param, int64_t id)
      : RequestObject(method_name, id) {
    _object.add(json_params_stirng, param);
    _param_type = ByIndex;
    _notifycation = false;
  }

  const string& method() { return _object.get_string(json_method_stirng); }
  const JsonObject& param_by_name() {
    if (_param_type != ByName) {
      // TODO throw
    }
    return _object.get_object(json_params_stirng);
  }
  const JsonArray& param_by_index() {
    if (_param_type != ByIndex) {
      // TODO throw
    }
    return _object.get_array(json_params_stirng);
  }
  int id() {
    if (_notifycation) {
      // TODO throw
    }
    return _object.get_int(json_id_stirng);
  }

  bool is_notifycation() { return _notifycation; }

  std::string to_string() { return _object.to_string(); }

  template <size_t I, typename... Args>
  void convert(std::tuple<Args...>& args) {
    convert<I>(args, typename detail::RecursiveTrait<I>::Tag());
  }
  template <size_t I, typename... Args>
  void convert(std::tuple<Args...>& args, const detail::NormalTag&) {
    JsonData data = _object.get_array(json_params_stirng).at(I);

    try {
      std::get<I>(args) =
          data.get<typename std::tuple_element<I, std::tuple<Args...>>::type>();
    } catch (const std::exception& e) {
      LOG_SYSFATAL << e.what();
    }

    convert<I - 1>(args, typename detail::RecursiveTrait<I - 1>::Tag());
  }

  template <size_t I, typename... Args>
  void convert(std::tuple<Args...>& args, const detail::EndTag&) {
    JsonData data = _object.get_array(json_params_stirng).at(I);
    std::get<I>(args) =
        data.get<typename std::tuple_element<I, std::tuple<Args...>>::type>();
    return;
  }

 private:
  enum ParamType { None, ByIndex, ByName };

  RequestObject() : _param_type(None), _notifycation(true) {
    _object.add(json_rpc_stirng, json_rpc_v2_stirng);
  }

  JsonObject _object;

  ParamType _param_type;
  bool _notifycation;
};

}  // namespace common
}  // namespace srpc

#endif