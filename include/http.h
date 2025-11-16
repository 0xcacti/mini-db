#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stddef.h>

#define HTTP_MAX_REQUEST_LEN 8192 * 4
#define HTTP_METHOD_MAX_LEN 8    // Based on maximum method length in HTTP/1.1
#define HTTP_PATH_MAX_LEN 2048   // Practical limit for URIs
#define HTTP_PROTOCOL_MAX_LEN 16 // Standard protocol length (e.g., HTTP/1.1)

typedef enum {
    HTTP_PARSE_OK,
    HTTP_PARSE_INVALID
} http_parse_e;

typedef enum {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_PATCH,
    HTTP_METHOD_UNK
} http_method_e;

typedef struct {
    char key[256];    // Header key (e.g., "Content-Type")
    char value[512];  // Header value (e.g., "text/html")
} http_header_t;

typedef struct {
    char method[HTTP_METHOD_MAX_LEN];
    http_method_e methode;
    char path[HTTP_PATH_MAX_LEN];
    char protocol[HTTP_PROTOCOL_MAX_LEN];
    char *buffer;
    http_header_t *headers;
    int header_count;
} http_request;


typedef struct {
    int status_code;
    char reason_phrase[64];
    http_header_t *headers;
    size_t header_count;
    char *body;
    size_t body_length;
} http_response;

http_method_e http_method_to_enum(const char *method_str);
http_parse_e read_http_request(int socket_fd, http_request *request);
void parse_http_headers(const char *buff, http_request *request);
void free_http_headers(http_request *request);

#endif // HTTP_H
