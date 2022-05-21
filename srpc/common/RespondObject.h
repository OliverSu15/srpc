#ifndef RESPOND_OBJECT_H
#define RESPOND_OBJECT_H
#include <climits>
#include <json.hpp>
#include <string>

#include "RequestObject.h"
namespace srpc {
namespace common {
static const std::string json_result_stirng = "result";
static const std::string json_error_stirng = "error";
static const std::string json_code_stirng = "code";
static const std::string json_message_stirng = "message";
static const std::string json_data_stirng = "data";

class ErrorObject {
 public:
  ErrorObject(int code, const std::string& message) {
    // TODO valid code
    _object.add(json_code_stirng, code);
    _object.add(json_message_stirng, message);
  }
  ErrorObject(int code, const std::string& message, const JsonData& data)
      : ErrorObject(code, message) {
    _object.add(json_data_stirng, data);
  }
  ErrorObject(int code, const std::string& message, const JsonArray& data)
      : ErrorObject(code, message) {
    _object.add(json_data_stirng, data);
  }
  ErrorObject(int code, const std::string& message, const JsonObject& data)
      : ErrorObject(code, message) {
    _object.add(json_data_stirng, data);
  }

  JsonObject get_json_object() { return _object; }

 private:
  JsonObject _object;
};

class RespondObject {
 public:
  explicit RespondObject(const JsonObject& object) : _object(object) {
    if (!_object.exist(json_rpc_stirng)) {
      throw std::invalid_argument("not a RPC respond");
    }
  }
  RespondObject(const JsonData& result, int id) : RespondObject(id) {
    _object.add(json_result_stirng, result);
  }
  RespondObject(const JsonArray& result, int id) : RespondObject(id) {
    _object.add(json_result_stirng, result);
  }
  RespondObject(const JsonObject& result, int id) : RespondObject(id) {
    _object.add(json_result_stirng, result);
  }

  RespondObject(ErrorObject error, int id) : RespondObject(id) {
    _object.add(json_error_stirng, std::move(error.get_json_object()));
    if (id == INT_MIN) {
      _object[json_id_stirng] = nullptr;
    }
  }

  std::string to_string() { return _object.to_string(); }

  const JsonObject& object() const { return _object; }

  int id() { return _object.get_int(json_id_stirng); }

  JsonData& result() { return _object[json_result_stirng]; }
  std::string& error_message() {
    return _object[json_error_stirng]
        .get_object()[json_message_stirng]
        .get_string();
  }
  int error_code() {
    return _object[json_error_stirng].get_object()[json_code_stirng].get_int();
  }
  bool is_error() { return _object.exist(json_error_stirng); }

 private:
  RespondObject() {
    init_head();
    _object.add(json_id_stirng, nullptr);
  };
  RespondObject(int id) {
    init_head();
    _object.add(json_id_stirng, id);
  };
  void init_head() { _object.add(json_rpc_stirng, json_rpc_v2_stirng); }

  JsonObject _object;
  bool _error;
};
}  // namespace common
}  // namespace srpc

#endif
