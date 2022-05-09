#ifndef REQUEST_OBJECT_H
#define REQUEST_OBJECT_H
#include <json.hpp>
#include <string>
#include <tuple>
#include <vector>

namespace srpc {
namespace common {
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
  // ctor for notifycation
  explicit RequestObject(const string& method_name) : RequestObject() {
    _object.add(json_method_stirng, method_name);
  }
  RequestObject(const string& method_name, const JsonObject& param)
      : RequestObject(method_name) {
    _object.add(json_params_stirng, param);
    _param_type = ByName;
  }
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
  RequestObject(const string& method_name, const JsonObject& param, int id)
      : RequestObject(method_name, id) {
    _object.add(json_params_stirng, param);
    _param_type = ByName;
    _notifycation = false;
  }
  RequestObject(const string& method_name, const JsonArray& param, int id)
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
    return _object.get_number(json_id_stirng);
  }

  bool is_notifycation() { return _notifycation; }

  std::string to_string() { return _object.to_string(); }

 private:
  enum ParamType { None, ByIndex, ByName };

  RequestObject() : _param_type(None), _notifycation(true) { init_head(); }
  void init_head() { _object.add(json_rpc_stirng, json_rpc_v2_stirng); }

  JsonObject _object;

  ParamType _param_type;
  bool _notifycation;
};
}  // namespace common
}  // namespace srpc

#endif