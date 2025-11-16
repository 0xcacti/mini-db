#include "http.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

http_parse_e read_http_request(int socket_fd, http_request *request) {
  char buffer[8192] = { 0 };
  ssize_t bytes_read = read(socket_fd, buffer, sizeof(buffer) - 1);

  if (bytes_read <= 0) {
    return HTTP_PARSE_INVALID;
  }

  buffer[bytes_read] = '\0';

  if (sscanf(buffer, "%7s %2047s %15s", request->method, request->path, request->protocol) != 3) {
    return HTTP_PARSE_INVALID;
  }

  return HTTP_PARSE_OK;
}

http_method_e http_method_to_enum(const char *method_str) {
  if (strcmp(method_str, "GET") == 0) {
    return HTTP_METHOD_GET;
  } else if (strcmp(method_str, "POST") == 0) {
    return HTTP_METHOD_POST;
  } else if (strcmp(method_str, "PUT") == 0) {
    return HTTP_METHOD_PUT;
  } else if (strcmp(method_str, "DELETE") == 0) {
    return HTTP_METHOD_DELETE;
  } else if (strcmp(method_str, "HEAD") == 0) {
    return HTTP_METHOD_HEAD;
  } else if (strcmp(method_str, "OPTIONS") == 0) {
    return HTTP_METHOD_OPTIONS;
  } else if (strcmp(method_str, "PATCH") == 0) {
    return HTTP_METHOD_PATCH;
  } else {
    return HTTP_METHOD_UNK;
  }
}

void free_http_headers(http_request *request) {
  return;
}

http_parse_e parse_http_headers(const char *buff, http_request *request) {
  return HTTP_PARSE_OK;
}
