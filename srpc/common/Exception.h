#ifndef RPC_EXCEPTION_H
#define RPC_EXCEPTION_H
#include <exception>
#include <string>
#include <utility>

#include "json.hpp"
#include "srpc/common/RequestObject.h"
#include "srpc/common/RespondObject.h"
namespace srpc {
namespace common {

class RpcException : public std::exception {
 public:
  RpcException(std::string message, int code = 0,
               common::JsonData data = nullptr)
      : _message(std::move(message)),
        _code(std::move(code)),
        _data(std::move(data)) {}
  ~RpcException() noexcept override = default;

  const char* what() const noexcept override { return _message.c_str(); }

  const std::string& message() const noexcept { return _message; }
  const int& code() const noexcept { return _code; }
  const common::JsonData& data() const noexcept { return _data; }

  common::ErrorObject get_error_object() const noexcept {
    return {_code, _message, _data};
  }

 private:
  std::string _message;
  int _code;
  common::JsonData _data;
};
}  // namespace common
}  // namespace srpc
#endif