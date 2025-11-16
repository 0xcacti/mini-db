#include "http.h"
#include <stdio.h>
#include <stdlib.h>
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

    // } else if (strcmp(method_str, "DELETE") == 0) {
    //   return HTTP_METHOD_DELETE;
    // } else if (strcmp(method_str, "HEAD") == 0) {
    //   return HTTP_METHOD_HEAD;
    // } else if (strcmp(method_str, "OPTIONS") == 0) {
    //   return HTTP_METHOD_OPTIONS;
    // } else if (strcmp(method_str, "PATCH") == 0) {
    //   return HTTP_METHOD_PATCH;
  } else if (strcmp(method_str, "UNKNOWN") == 0) {
    return HTTP_METHOD_UNK;
  } else {
    return HTTP_METHOD_UNK;
  }
}

http_parse_e parse_http_headers(const char *raw_request, http_request *request) {
  if (request == NULL || raw_request == NULL) {
    return HTTP_PARSE_INVALID;
  }

  request->buffer = NULL;
  request->headers = NULL;
  request->header_count = 0;

  size_t req_len = strlen(raw_request);
  request->buffer = malloc(req_len + 1);
  if (request->buffer == NULL) {
    return HTTP_PARSE_INVALID;
  }
  memcpy(request->buffer, raw_request, req_len + 1);

  const char *line_end = strstr(raw_request, "\r\n");
  if (line_end == NULL) {
    return HTTP_PARSE_INVALID;
  }

  size_t request_line_len = (size_t)(line_end - raw_request);
  char request_line[1024];
  if (request_line_len >= sizeof(request_line)) {
    return HTTP_PARSE_INVALID;
  }
  memcpy(request_line, raw_request, request_line_len);
  request_line[request_line_len] = '\0';

  if (sscanf(request_line, "%7s %2047s %15s", request->method, request->path, request->protocol) !=
      3) {
    return HTTP_PARSE_INVALID;
  }

  request->methode = http_method_to_enum(request->method);

  const char *line_start = line_end + 2;
  while (line_start != NULL && *line_start != '\0' && *line_start != '\r' && *line_start != '\n') {
    const char *next_end = strstr(line_start, "\r\n");
    if (next_end == NULL) {
      break;
    }

    size_t line_length = (size_t)(next_end - line_start);
    if (line_length == 0) {
      break;
    }

    char line[1024];
    if (line_length >= sizeof(line)) {
      line_length = sizeof(line) - 1;
    }
    memcpy(line, line_start, line_length);
    line[line_length] = '\0';

    char *colon_pos = strchr(line, ':');
    if (colon_pos != NULL) {
      *colon_pos = '\0';
      const char *key = line;
      const char *value = colon_pos + 1;

      while (*value == ' ') {
        value++;
      }

      http_header_t *new_headers =
          realloc(request->headers, sizeof(http_header_t) * (request->header_count + 1));
      if (new_headers == NULL) {
        return HTTP_PARSE_INVALID;
      }
      request->headers = new_headers;

      http_header_t *hdr = &request->headers[request->header_count];

      hdr->key[0] = '\0';
      hdr->value[0] = '\0';

      strncpy(hdr->key, key, sizeof(hdr->key) - 1);
      hdr->key[sizeof(hdr->key) - 1] = '\0';

      strncpy(hdr->value, value, sizeof(hdr->value) - 1);
      hdr->value[sizeof(hdr->value) - 1] = '\0';

      request->header_count++;
    }

    line_start = next_end + 2;
  }

  return HTTP_PARSE_OK;
}

void free_http_headers(http_request *request) {
  (void)request;
}
