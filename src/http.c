#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

http_parse_e read_http_request(int socket_fd, http_request *request) {
  memset(request, 0, sizeof(*request));

  size_t total = 0;
  while (total < HTTP_MAX_REQUEST_LEN - 1) {
    ssize_t n = read(socket_fd, request->buffer + total, HTTP_MAX_REQUEST_LEN - 1 - total);
    if (n < 0) {
      return HTTP_PARSE_INVALID;
    }
    if (n == 0) {
      break;
    }
    total += (size_t)n;

    if (strstr(request->buffer, "\r\n\r\n") != NULL) {
      break;
    }
  }

  if (total == 0) {
    return HTTP_PARSE_INVALID;
  }

  request->buffer[total] = '\0';

  if (sscanf(
          request->buffer, "%7s %2047s %15s", request->method, request->path, request->protocol) !=
      3) {
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
  const char *line_start = strstr(raw_request, "\r\n");
  if (!line_start) return HTTP_PARSE_INVALID;

  line_start += 2; // Skip the request line
  while (line_start && *line_start && *line_start != '\r' && *line_start != '\n') {
    const char *line_end = strstr(line_start, "\r\n");
    if (!line_end) break;

    size_t line_length = line_end - line_start;
    char line[1024] = { 0 };
    strncpy(line, line_start, line_length);

    char *colon_pos = strchr(line, ':');
    if (colon_pos) {
      *colon_pos = '\0';
      const char *key = line;
      const char *value = colon_pos + 1;

      while (*value == ' ')
        value++; // Skip leading spaces

      request->headers =
          realloc(request->headers, sizeof(http_header_t) * (request->header_count + 1));
      if (!request->headers) {
        perror("Failed to allocate memory for headers");
        exit(EXIT_FAILURE);
      }

      strncpy(request->headers[request->header_count].key,
              key,
              sizeof(request->headers[request->header_count].key) - 1);
      strncpy(request->headers[request->header_count].value,
              value,
              sizeof(request->headers[request->header_count].value) - 1);

      request->header_count++;
    }

    line_start = line_end + 2;
  }

  return HTTP_PARSE_OK;
}

void free_http_headers(http_request *request) {
  free(request->headers);
  request->headers = NULL;
  request->header_count = 0;
}

http_parse_e parse_http_request(const char *raw_request, http_request *request) {
  // memset(request, 0, sizeof(*request));

  size_t len = strlen(raw_request);
  if (len >= HTTP_MAX_REQUEST_LEN) {
    len = HTTP_MAX_REQUEST_LEN - 1;
  }
  memcpy(request->buffer, raw_request, len);
  request->buffer[len] = '\0';

  if (sscanf(
          request->buffer, "%7s %2047s %15s", request->method, request->path, request->protocol) !=
      3) {
    return HTTP_PARSE_INVALID;
  }

  request->methode = http_method_to_enum(request->method);

  if (parse_http_headers(request->buffer, request) != HTTP_PARSE_OK) {
    free_http_headers(request);
    return HTTP_PARSE_INVALID;
  }

  memcpy(request->buffer, raw_request, sizeof(request->buffer) - 1);
  request->buffer[sizeof(request->buffer) - 1] = '\0';
  return HTTP_PARSE_OK;
}

void init_http_response(http_response *response) {
  response->status_code = 200;
  strncpy(response->reason_phrase, "OK", sizeof(response->reason_phrase) - 1);
  response->headers = NULL;
  response->header_count = 0;
  response->body = NULL;
  response->body_length = 0;
}

void add_http_header(http_response *response, const char *key, const char *value) {
  response->headers =
      realloc(response->headers, sizeof(http_header_t) * (response->header_count + 1));
  if (!response->headers) {
    perror("Failed to allocate memory for response headers");
    exit(EXIT_FAILURE);
  }

  strncpy(response->headers[response->header_count].key,
          key,
          sizeof(response->headers[response->header_count].key) - 1);
  strncpy(response->headers[response->header_count].value,
          value,
          sizeof(response->headers[response->header_count].value) - 1);

  response->header_count++;
}

void free_http_response(http_response *response) {
  free(response->headers);
  response->headers = NULL;
  response->header_count = 0;
  free(response->body);
  response->body = NULL;
  response->body_length = 0;
}
