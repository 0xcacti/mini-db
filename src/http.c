#include "http.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int read_http_request(int socket_fd, http_request *request) {
  char buffer[8192] = { 0 };
  ssize_t bytes_read = read(socket_fd, buffer, sizeof(buffer) - 1);

  if (bytes_read <= 0) {

    return -1; // Reading failed or connection closed
  }

  buffer[bytes_read] = '\0';

  if (sscanf(buffer, "%7s %2047s %15s", request->method, request->path, request->protocol) != 3) {
    return -1; // Failed to parse the request line
  }

  return 0;
}
