#include "common.h"
#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int send_hello(int fd) {
  char buf[4096] = { 0 };

  dbproto_hdr_t *hdr = (dbproto_hdr_t *)buf;
  hdr->type = htonl((uint16_t)MSG_HELLO_REQ);
  hdr->length = htons((uint16_t)1);

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
  char *addarg = NULL;
  char *portarg = NULL;
  char *hostarg = NULL;
  unsigned short port = 0;

  int c;
  while ((c = getopt(argc, argv, "p:h:a:")) != -1) {
    switch (c) {
    case 'a':
      addarg = optarg;
      break;
    case 'p':
      portarg = optarg;
      port = (unsigned short)atoi(portarg);
      break;
    case 'h':
      hostarg = optarg;
      break;
    case '?':
      printf("Unknown option: %c\n", optopt);
    default:
      return 1;
    }
  }

  if (port == 0) {
    printf("Port must be specified with -p\n");
    return 1;
  }

  if (hostarg == NULL) {
    printf("Host must be specified with -h\n");
    return 1;
  }

  struct sockaddr_in server_info = { 0 };
  server_info.sin_family = AF_INET;
  server_info.sin_addr.s_addr = inet_addr(hostarg);
  server_info.sin_port = htons(port);

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

  if (send_hello(fd) != STATUS_SUCCESS) {
    close(fd);
    exit(EXIT_FAILURE);
  }

  close(fd);
  return 0;
}
