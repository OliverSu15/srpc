#include <cstdio>

#include "srpc/client/RpcClient.h"
#include "srpc/client/RpcClientService.h"
#include "suduo/base/Timestamp.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/InetAddress.h"
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