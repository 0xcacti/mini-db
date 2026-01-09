// #include <pthread.h>
// #include <stdio.h>
// #include <stdlib.h>
//
// // OG way
// pthread_key_t key;
//
// void array_destructor(void *array) {
//   free(array);
//   printf("Array freed for a thread \n");
// }
//
// void *thread_function(void *arg) {
//   int *my_array = (int *)malloc(10 * sizeof(int));
//   for (int i = 0; i < 10; i++) {
//     my_array[i] = i;
//   }
//
//   pthread_setspecific(key, my_array);
//
//   for (int i = 0; i < 10; i++) {
//     printf("%d ", my_array[i]);
//   }
//   printf("\n");
//
//   return NULL;
// }
//
// int main() {
//   pthread_t thread1, thread2;
//
//   pthread_key_create(&key, array_destructor);
//
//   pthread_create(&thread1, NULL, thread_function, NULL);
//   pthread_create(&thread2, NULL, thread_function, NULL);
//
//   pthread_join(thread1, NULL);
//   pthread_join(thread2, NULL);
//
//   pthread_key_delete(key);
//   return 0;
// }

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

__thread int tls_var = 0;

void *thread_function(void *arg) {
  tls_var = (int)(long)arg;
  printf("Thread %ld has TLS variable %d\n", pthread_self(), tls_var);
  sleep(1);
  return NULL;
}

int main() {
  pthread_t tid1, tid2;

  pthread_create(&tid1, NULL, thread_function, (void *)1);
  pthread_create(&tid2, NULL, thread_function, (void *)2);

  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);

  return 0;
}
