#include "tcp.h"
#include <stdio.h>

server_status_e bind_tcp_port(tcp_server *server, int port) {
  memset(server, 0, sizeof(*server));
  server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->socket_fd == -1) {
    perror("socket");
    return SERVER_SOCKET_ERROR;
  }

  server->address.sin_family = AF_INET;
  server->address.sin_addr.s_addr = INADDR_ANY;
  server->address.sin_port = htons(port);

  if (port < 0 || port > 65535) {
    fprintf(stderr, "Invalid port number: %d\n", port);
    close(server->socket_fd);
    return SERVER_BIND_ERROR;
  }

  if (bind(server->socket_fd, (struct sockaddr *)&server->address, sizeof(server->address))) {
    perror("bind");
    close(server->socket_fd);
    return SERVER_BIND_ERROR;
  }

  if (listen(server->socket_fd, 5) < 0) {
    perror("listen");
    close(server->socket_fd);
    return SERVER_LISTEN_ERROR;
  }

  printf("Server bound and listening\n");
  return SERVER_OK;
}

int accept_client(int server_fd) {
  struct sockaddr_in client_address = { 0 };
  socklen_t client_len = sizeof(client_address);

  int client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_len);
  if (client_fd < 0) {
    perror("accept");
    return -1;
  }

  return client_fd;
}
