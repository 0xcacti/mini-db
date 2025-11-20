#include "route.h"
#include "http.h"
#include <stdlib.h>

route_t routes[MAX_ROUTES];
size_t route_count = 0;

size_t install_route(http_method_e method,
                     const char *path,
                     void (*handler)(http_request *req, http_response *res)) {
  if (route_count >= MAX_ROUTES) {
    exit(EXIT_FAILURE);
  }
  routes[route_count].methode = method;
  strcpy(routes[route_count].path, path);
  routes[route_count].handler = handler;
  return ++route_count;
}
