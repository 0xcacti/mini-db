#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int send_hello(int fd) {
  char buf[4096] = { 0 };

  dbproto_hdr_t *hdr = (dbproto_hdr_t *)buf;
  hdr->type = htonl((uint16_t)MSG_HELLO_REQ);
  hdr->length = htons(1);

  dbproto_hello_req_t *req = (dbproto_hello_req_t *)&hdr[1];
  req->proto = htons(PROTO_VER);

  write(fd, buf, sizeof(dbproto_hdr_t) + sizeof(dbproto_hello_req_t));
  read(fd, buf, sizeof(buf));
  hdr->type = ntohl(hdr->type);
  hdr->length = ntohs(hdr->length);
  if (hdr->type == MSG_ERROR) {
    printf("err: protocol mismatch");
    close(fd);
    return STATUS_ERROR;
  }

  printf("Connected to server on protocol: v%d\n", PROTO_VER);
  return STATUS_SUCCESS;
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

  int s = send_hello(fd);
  if (s != 0) {
    close(fd);
    exit(EXIT_FAILURE);
  }
  close(fd);

  return 0;
}
