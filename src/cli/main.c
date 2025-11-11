#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void send_hello(int fd) {
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <ip of the host>\n", argv[0]);
    return 1;
  }

  struct sockaddr_in server_info = { 0 };
  server_info.sin_family = AF_INET;
  server_info.sin_addr.s_addr = inet_addr(argv[1]);
  server_info.sin_port = htons(8080);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  if (connect(fd, (struct sockaddr *)&server_info, sizeof(server_info)) == -1) {
    perror("connect");
    close(fd);
    exit(EXIT_FAILURE);
  }

  send_helo(fd);
  close(fd);
  return 0;
}
