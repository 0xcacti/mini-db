#include "http.h"
#include "tcp.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int port;
} server_config_t;

int loadConfig(server_config_t *config) {
  int status = 0;

  char *configdata = loadfile("config.json");
  if (!configdata) {
    fprintf(stderr, "Failed to load config, what happened?");
  }

  cJSON *config_json = cJSON_Parse(configdata);
  if (config_json == NULL) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    status = 0;
    goto end;
  }

  cJSON *port = cJSON_GetObjectItemCaseSensitive(config_json, "portnumber");
  if (!cJSON_IsNumber(port)) {
    status = 0;
    goto end;
  }

  if (port->valueint > 65535 || port->valueint < 0) {
    fprintf(stderr, "Invalid port number specified in config.");
    status = 0;
    goto end;
  }

  config->port = (short)port->valueint;

end:
  cJSON_Delete(config_json);
  return status;
}

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
