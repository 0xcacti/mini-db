#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>

int main() {

  threadpool_t *pool = malloc(sizeof(threadpool_t));
  if (pool == NULL) {
    fprintf(stderr, "Failed to allocate memory for thread pool\n");
    return EXIT_FAILURE;
  }
  threadpool_init(pool);

  free(pool);
}
