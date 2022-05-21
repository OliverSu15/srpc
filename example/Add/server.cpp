#include "json.hpp"
#include "srpc/server/RpcServer.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/InetAddress.h"
#include "suduo/net/TcpServer.h"
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