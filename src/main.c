#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  tcp_server_t server = { 0 };
  server_status_e status = bind_tcp_port(&server, 8080);
  if (status != SERVER_OK) {
    fprintf(stderr, "server init failed\n");
    return 1;
  }

  int client_fd = accept_client(server.socket_fd);
  if (client_fd == -1) {
    fprintf(stderr, "failed to accept client connection\n");
    close(server.socket_fd);
    return 1;
  }
  printf("client connected\n");

  close(client_fd);
  close(server.socket_fd);

  return 0;
}
