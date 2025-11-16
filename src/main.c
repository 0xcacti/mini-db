#include "http.h"
#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  tcp_server server = { 0 };
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

  http_request request = { 0 };

  if (read_http_request(client_fd, &request) != HTTP_PARSE_OK) {
    fprintf(stderr, "failed to read http request\n");
    close(client_fd);
    close(server.socket_fd);
    return 1;
  }

  printf("Received HTTP request:\n");
  printf("Method: %s\n", request.method);
  printf("Path: %s\n", request.path);
  printf("Protocol: %s\n", request.protocol);

  close(client_fd);
  close(server.socket_fd);

  return 0;
}
