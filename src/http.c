#include "http.h"
#include "route.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

extern route_t routes[];
extern int route_count;

bool handle_route(http_request *request, http_response *response) {
  for (int i = 0; i < route_count; i++) {
    if (routes[i].methode == request->methode && strcmp(routes[i].path, request->path) == 0) {
      routes[i].handler(request, response);
      return true;
    }
  }
  return false;
}

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

char *construct_http_response(const http_response *response, size_t *response_length) {
  size_t buffer_size = 1024;
  char *buffer = malloc(buffer_size);
  if (!buffer) {
    perror("Failed to allocate memory for response");
    exit(EXIT_FAILURE);
  }

  size_t offset = snprintf(
      buffer, buffer_size, "HTTP/1.1 %d %s\r\n", response->status_code, response->reason_phrase);

  for (size_t i = 0; i < response->header_count; i++) {
    size_t header_len =
        snprintf(NULL, 0, "%s: %s\r\n", response->headers[i].key, response->headers[i].value);
    while (offset + header_len + 1 > buffer_size) {
      buffer_size += 2;
      buffer = realloc(buffer, buffer_size);
      if (!buffer) {
        perror("Failed to reallocate memory");
        exit(EXIT_FAILURE);
      }
    }
    offset += snprintf(buffer + offset,
                       buffer_size - offset,
                       "%s: %s\r\n",
                       response->headers[i].key,
                       response->headers[i].value);
  }

  offset += snprintf(buffer + offset, buffer_size - offset, "\r\n");

  if (response->body) {
    while (offset + response->body_length + 1 > buffer_size) {
      buffer_size += 2;
      buffer = realloc(buffer, buffer_size);
      if (!buffer) {
        perror("Failed to reallocate memory");
        exit(EXIT_FAILURE);
      }
    }
    memcpy(buffer + offset, response->body, response->body_length);
    offset += response->body_length;
  }
  *response_length = offset;
  return buffer;
}

void send_http_response(int client_fd, const http_response *response) {
  size_t response_length = 0;
  char *response_data = construct_http_response(response, &response_length);

  size_t total_sent = 0;
  while (total_sent < response_length) {
    ssize_t bytes_sent =
        send(client_fd, response_data + total_sent, response_length - total_sent, 0);
    if (bytes_sent <= 0) {
      perror("send");
      break;
    }
    total_sent += bytes_sent;
  }
  free(response_data);
}

char *loadfile(const char *path) {
  FILE *f = fopen(path, "rb+");
  if (f == NULL) {
    return NULL;
  }

  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return NULL;
  }

  long len = ftell(f);
  if (len < 0) {
    fclose(f);
    return NULL;
  }

  if (fseek(f, 0, SEEK_SET) != 0) {
    fclose(f);
    return NULL;
  }

  char *data = malloc((size_t)len + 1);
  if (data == NULL) {
    fclose(f);
    return NULL;
  }

  size_t n = fread(data, 1, (size_t)len, f);
  fclose(f);

  if (n != (size_t)len) {
    free(data);
    return NULL;
  }

  data[len] = '\0';
  return data;
}

void serve_file(const char *path, http_response *response) {
  char *file_content = loadfile(path);
  if (file_content == NULL) {
    response->status_code = 404;
    strncpy(response->reason_phrase, "Not Found", sizeof(response->reason_phrase) - 1);
    return serve_file("./www/404.html", response);
  }

  response->body = file_content;
  size_t file_size = strlen(file_content);
  response->body_length = file_size;

  if (strstr(path, ".html")) {
    add_http_header(response, "Content-Type", "text/html");
  } else if (strstr(path, ".css")) {
    add_http_header(response, "Content-Type", "text/css");
  } else if (strstr(path, ".js")) {
    add_http_header(response, "Content-Type", "application/javascript");
  } else if (strstr(path, ".png")) {
    add_http_header(response, "Content-Type", "image/png");
  } else {
    add_http_header(response, "Content-Type", "application/octet-stream");
  }

  char content_length[32];
  snprintf(content_length, sizeof(content_length), "%zu", file_size);
  add_http_header(response, "Content-Length", content_length);

  free(file_content);
}

void sanitize_path(const char *requested_path, char *sanitized_path, size_t buffer_size) {
  const char *web_root = "./www";
  snprintf(sanitized_path, buffer_size, "%s%s", web_root, requested_path);

  if (strstr(sanitized_path, "..")) {
    strncpy(sanitized_path, "./www/404.html", buffer_size - 1);
  }
}
