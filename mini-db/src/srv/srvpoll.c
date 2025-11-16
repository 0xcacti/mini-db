#include "srvpoll.h"
#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void init_clients(clientstate_t *states) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    states[i].fd = -1; // -1 indicates a free slot
    states[i].state = STATE_NEW;
    memset(&states[i].buffer, '\0', BUFF_SIZE);
  }
}

int find_free_slot(clientstate_t *states) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (states[i].fd == -1) {
      return i;
    }
  }
  return -1; // No free slot found
}

int find_slot_by_fd(clientstate_t *states, int fd) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (states[i].fd == fd) {
      return i;
    }
  }
  return -1; // Not found
}

void fsm_reply_hello(clientstate_t *client, dbproto_hdr_t *hdr) {
  hdr->type = htonl((uint16_t)MSG_HELLO_RESP);
  hdr->length = htons(1);

  dbproto_hello_resp_t *resp = (dbproto_hello_resp_t *)&hdr[1];
  resp->proto = htons(PROTO_VER);

  write(client->fd, client->buffer, sizeof(dbproto_hdr_t) + sizeof(dbproto_hello_resp_t));
}

void fsm_reply_hello_err(clientstate_t *client, dbproto_hdr_t *hdr) {
  hdr->type = htonl((uint16_t)MSG_ERROR);
  hdr->length = htons(1);

  write(client->fd, client->buffer, sizeof(dbproto_hdr_t));
}

void fsm_reply_add_err(clientstate_t *client, dbproto_hdr_t *hdr) {
  hdr->type = htonl((uint16_t)MSG_ERROR);
  hdr->length = htons(1);

  write(client->fd, client->buffer, sizeof(dbproto_hdr_t));
}

void fsm_reply_add(clientstate_t *client, dbproto_hdr_t *hdr) {
  hdr->type = htonl((uint16_t)MSG_EMPLOYEE_ADD_RESP);
  hdr->length = htons(1);

  write(client->fd, client->buffer, sizeof(dbproto_hdr_t));
}

void send_employees(struct dbheader_t *dbhdr,
                    struct employee_t **employees,
                    clientstate_t *client) {

  dbproto_hdr_t *hdr = (dbproto_hdr_t *)client->buffer;
  hdr->type = htonl((uint16_t)MSG_EMPLOYEE_LIST_RESP);
  hdr->length = htons((uint16_t)dbhdr->count);

  write(client->fd, client->buffer, sizeof(dbproto_hdr_t));
  dbproto_employee_list_resp_t *current_employee = (dbproto_employee_list_resp_t *)(hdr + 1);
  struct employee_t *employees_array = *employees;

  for (int i = 0; i < dbhdr->count; i++) {
    strncpy(current_employee->name, employees_array[i].name, sizeof(current_employee->name) - 1);
    strncpy(current_employee->address,
            employees_array[i].address,
            sizeof(current_employee->address) - 1);
    current_employee->hours = htonl(employees_array[i].hours);
    write(client->fd, current_employee, sizeof(dbproto_employee_list_resp_t));
  }
}

void handle_client_fsm(int dbfd,
                       struct dbheader_t *dbhdr,
                       struct employee_t **employees,
                       clientstate_t *client) {
  dbproto_hdr_t *hdr = (dbproto_hdr_t *)client->buffer;
  hdr->type = ntohl(hdr->type);
  hdr->length = ntohl(hdr->length);

  if (client->state == STATE_HELLO) {
    if (hdr->type != MSG_HELLO_REQ || hdr->length == 1) {
      printf("Didn't get MSG_HELLO_REQ\n");
    }

    dbproto_hello_req_t *hello_req = (dbproto_hello_req_t *)&hdr[1];
    hello_req->proto = ntohs(hello_req->proto);
    if (hello_req->proto != PROTO_VER) {
      printf("Protocol mismatch\n");
      fsm_reply_hello_err(client, hdr);
      return;
    }

    fsm_reply_hello(client, hdr);
    client->state = STATE_MSG;
    printf("Client upgraded to STATE_MSG\n");
  }

  if (client->state == STATE_MSG) {
    if (hdr->type == MSG_EMPLOYEE_ADD_REQ) {
      dbproto_employee_add_req_t *add_req = (dbproto_employee_add_req_t *)&hdr[1];
      printf("Adding employee: %s\n", add_req->data);
      if (add_employee(dbhdr, employees, (char *)add_req->data) != STATUS_SUCCESS) {
        fsm_reply_add_err(client, hdr);
      } else {
        fsm_reply_add(client, hdr);
        output_file(dbfd, dbhdr, *employees);
      }
    }

    if (hdr->type == MSG_EMPLOYEE_LIST_REQ) {
      printf("Listing employees\n");
      send_employees(dbhdr, employees, client);
    }
  }
}
