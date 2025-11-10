#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"
#include "srvpoll.h"

#define MAX_CLIENTS 256

clientstate_t clients[MAX_CLIENTS] = { 0 };

void print_usage(char *argv[]) {
  printf("Usage: %s -n -f <database file>\n", argv[0]);
  printf("\t-n  - Create a new database file\n");
  printf("\t-f  - (Required) Specify the database file to use\n");
  printf("\t-p  - (Required) Specify the port to listen on\n");
  return;
}

void poll_loop(unsigned short port, struct dbheader_t *dbhdr, struct employee_t *employees) {
  int listen_fd, conn_fd, free_slot;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);

  struct pollfd fds[MAX_CLIENTS + 1];
  int nfds = 1;
  int opt = 1;

  init_clients(clients);

  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt failed");
    close(listen_fd);
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    close(listen_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(listen_fd, 10) < 0) {
    perror("Listen failed");
    close(listen_fd);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d\n", port);

  memset(fds, 0, sizeof(fds));
  fds[0].fd = listen_fd;
  fds[0].events = POLLIN;
  nfds = 1;

  while (1) {
    int ii = 1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (clients[i].fd != -1) {
        fds[ii].fd = clients[i].fd;
        fds[ii].events = POLLIN;
        ii++;
      }
    }
    int n_events = poll(fds, nfds, -1);
    if (n_events < 0) {
      perror("poll");
      exit(EXIT_FAILURE);
    }

    if (fds[0].revents & POLLIN) {
      if ((conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
        perror("Accept failed");
        continue;
      }
      printf("New connection from %s:%d, fd=%d\n",
             inet_ntoa(client_addr.sin_addr),
             ntohs(client_addr.sin_port),
             conn_fd);
      free_slot = find_free_slot(clients);
      if (free_slot != -1) {
        clients[free_slot].fd = conn_fd;
        clients[free_slot].state = STATE_CONNECTED;
        memset(clients[free_slot].buffer, '\0', BUFF_SIZE);
      } else {
        printf("Max clients reached, rejecting connection\n");
        close(conn_fd);
      }
      n_events--;
    }

    for (int i = 1; i < nfds && n_events > 0; i++) {

      if (fds[i].revents & POLLIN) {
        n_events--;
        int fd = fds[i].fd;
        int slot = find_slot_by_fd(clients, fd);
        ssize_t bytes_read = read(fd, &clients[slot].buffer, sizeof(clients[slot].buffer));
        if (bytes_read <= 0) {
          close(fd);
          if (slot == -1) {
            printf("Tried to close unknown fd %d\n", fd);
          } else {
            clients[slot].fd = -1;
            clients[slot].state = STATE_DISCONNECTED;
            printf("Client on fd %d disconnected\n", fd);
            nfds--;
          }
        } else {
          printf("Received %zd bytes from fd %d\n", bytes_read, fd);
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  char *filepath = NULL;
  char *portarg = NULL;
  unsigned short port = 0;
  bool newfile = false;
  int c = 0;

  int dbfd = -1;
  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  while ((c = getopt(argc, argv, "nf:p:")) != -1) {
    switch (c) {
    case 'n':
      newfile = true;
      break;
    case 'f':
      filepath = optarg;
      break;
    case 'p':
      portarg = optarg;
      port = atoi(portarg);
      if (port == 0) {
        printf("Invalid port number: %s\n", portarg);
      }
      break;
    case '?':
      printf("Unknown option: -%c\n", optopt);
    default:
      return -1;
    }
  }

  if (filepath == NULL) {
    printf("filepath and port are required arguments.\n");
    print_usage(argv);
    return -1;
  }

  if (port == 0) {
    printf("A valid port number is required. Received: %d\n", port);
    print_usage(argv);
    return -1;
  }

  if (newfile) {
    dbfd = create_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to create database file\n");
      return -1;
    }

    if (create_db_header(&dbhdr) == STATUS_ERROR) {
      printf("Unable to create database header\n");
      return -1;
    }
  } else {
    dbfd = open_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to open database file\n");
      return -1;
    }

    if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Failed to validate database header\n");
      return -1;
    }
  }

  if (read_employees(dbfd, dbhdr, &employees) == STATUS_ERROR) {
    printf("Failed to read employees");
    return -1;
  }
}
