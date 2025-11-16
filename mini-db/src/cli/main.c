#include "common.h"
#include <arpa/inet.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int send_employee(int fd, char *addarg) {
  char buf[4096] = { 0 };
  dbproto_hdr_t *hdr = (dbproto_hdr_t *)buf;
  hdr->type = htonl((uint16_t)MSG_EMPLOYEE_ADD_REQ);
  hdr->length = htons((uint16_t)1);

  dbproto_employee_add_req_t *employee = (dbproto_employee_add_req_t *)(hdr + 1);
  size_t input_len = strlen(addarg);
  if (input_len > sizeof employee->data) {
    input_len = sizeof employee->data;
  }
  memcpy(employee->data, addarg, input_len);
  write(fd, hdr, sizeof(dbproto_hdr_t) + sizeof(dbproto_employee_add_req_t));
  read(fd, buf, sizeof(buf));

  hdr->type = ntohl(hdr->type);
  hdr->length = ntohl(hdr->length);
  if (hdr->type == MSG_ERROR) {
    printf("improper format for add employee string\n");
    close(fd);
    return STATUS_ERROR;
  }

  if (hdr->type == MSG_EMPLOYEE_ADD_RESP) {
    printf("Employee added successfully\n");
  }

  return STATUS_SUCCESS;
}

int list_employees(int fd) {
  char buf[4096] = { 0 };
  dbproto_hdr_t *hdr = (dbproto_hdr_t *)buf;
  hdr->type = htonl((uint16_t)MSG_EMPLOYEE_LIST_REQ);
  hdr->length = htons((uint16_t)0);

  write(fd, hdr, sizeof(dbproto_hdr_t));
  read(fd, buf, sizeof(buf));

  hdr->type = ntohl(hdr->type);
  hdr->length = ntohs(hdr->length);

  if (hdr->type == MSG_ERROR) {
    printf("error listing employees\n");
    close(fd);
    return STATUS_ERROR;
  }

  if (hdr->type == MSG_EMPLOYEE_LIST_RESP) {
    printf("Employee List:\n");
    dbproto_employee_list_resp_t *employee = (dbproto_employee_list_resp_t *)(hdr + 1);
    for (int i = 0; i < hdr->length; i++) {
      read(fd, employee, sizeof(dbproto_employee_list_resp_t));
      printf("%s, %s, %d\n", employee->name, employee->address, ntohl(employee->hours));
    }
  }

  return STATUS_SUCCESS;
}

int main(int argc, char *argv[]) {
  char *addarg = NULL;
  char *portarg = NULL;
  char *hostarg = NULL;
  bool list = false;
  unsigned short port = 0;

  int c;
  while ((c = getopt(argc, argv, "p:h:a:l")) != -1) {
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
    case 'l':
      list = true;
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

  if (addarg && send_employee(fd, addarg) != STATUS_SUCCESS) {
    close(fd);
    exit(EXIT_FAILURE);
  }

  if (list && list_employees(fd) != STATUS_SUCCESS) {
    close(fd);
    exit(EXIT_FAILURE);
  }

  close(fd);
  return 0;
}
