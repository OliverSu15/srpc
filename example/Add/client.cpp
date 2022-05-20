#include <cstdio>

#include "srpc/client/RpcClient.h"
#include "srpc/client/RpcClientService.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/InetAddress.h"
int main() {
  suduo::net::EventLoop loop;
  suduo::net::InetAddress addr(2000, false);
  srpc::client::RpcClient client(&loop, addr);

  srpc::client::RpcClientService service;
  client.register_service(service);

  loop.run_every(5, [&service]() {
    LOG_INFO << "here";
    service.call_procedure(
        "add", [](s2ujson::JSON_Data& data) { LOG_INFO << data.get_int(); }, 1,
        2);
  });

  client.connect();
  loop.loop();
}