# srpc
`srpc`是一个没有代码生成，使用JSON的C++ RPC库。

## 简介
`srpc`是一个没有代码生成，使用JSON的C++ RPC库。

它的传输协议使用的是`JSON-RPC 2.0`，但是为了不生成代码，所以原版的按名字传递参数（`"lhs":1`这类的）没有实现。

`srpc`使用 [suduo](https://github.com/OliverSu15/suduo) 作为其网络库，[s2ujson](https://github.com/OliverSu15/s2ujson) 作为其JSON库

## 使用
服务端
```cpp
int add(int left, int right) { return left + right; }

int main() {
  suduo::net::EventLoop loop;
  suduo::net::InetAddress addr(2000, false);
  srpc::server::RpcServer server(&loop, addr);

  auto service = server.create_service();

  service->bind_procedure("add", &add);

  server.start();
  loop.loop();
}
```
客户端
```cpp
int main() {
  suduo::net::EventLoop loop;
  suduo::net::InetAddress addr(2000, false);
  srpc::client::RpcClient client(&loop, addr);

  srpc::client::RpcClientService service;
  client.register_service(service);

  loop.run_after(10, [&service]() {
    LOG_INFO << "start";
    service.call_procedure(
        "add", [](s2ujson::JSON_Data& data) { LOG_INFO << data.get_int(); }, 1,
        2);
    service.call_procedure(
        "Add", [](s2ujson::JSON_Data& data) { LOG_INFO << data.get_int(); }, 1,
        2);  // error test
    srpc::client::Batch batch;
    batch.add_procedure(
        "add", [](s2ujson::JSON_Data& data) { LOG_INFO << data.get_int(); }, 2,
        3);
    batch.add_procedure(
        "add", [](s2ujson::JSON_Data& data) { LOG_INFO << data.get_int(); }, 5,
        5);
    service.call_batch(batch);
    LOG_INFO << "end";
  });

  client.connect();
  loop.loop();
}
```

## 环境
`srpc`是在以下的环境里构建的：

系统：WSL2 Arch Linux

编译器：GCC 11.2.0

构造工具：cmake 3.22.2

C++17

## 致谢
感谢 [rpclib](https://github.com/rpclib/rpclib) 项目

感谢 [libjson-rpc-cpp](https://github.com/cinemast/libjson-rpc-cpp) 项目

感谢 [jrpc](https://github.com/guangqianpeng/jrpc) 项目