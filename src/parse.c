#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees,
                 char *addstring) {
  printf("addString: %s\n", addstring);
  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr,
                   struct employee_t **employeesOut) {
  if (fd < 0) {
    perror("Invalid file descriptor");
    return STATUS_ERROR;
  }
  int count = dbhdr->count;
  struct employee_t *employees = calloc(count, sizeof(struct employee_t));
  if (!employees) {
    perror("calloc");
    return STATUS_ERROR;
  }

  if (read(fd, employees, count * sizeof(struct employee_t)) !=
      count * sizeof(struct employee_t)) {
    perror("read");
    free(employees);
    return STATUS_ERROR;
  }

  for (int i = 0; i < count; i++) {
    employees[i].hours = ntohl(employees[i].hours);
  }

  employeesOut = &employees;

  return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr,
                struct employee_t *employees) {
  if (fd < 0) {
    perror("Invalid file descriptor");
    return STATUS_ERROR;
  }

  dbhdr->magic = htonl(dbhdr->magic);
  dbhdr->filesize = htonl(dbhdr->filesize);
  dbhdr->count = htons(dbhdr->count);
  dbhdr->version = htons(dbhdr->version);

  lseek(fd, 0, SEEK_SET);
  write(fd, dbhdr, sizeof(struct dbheader_t));
  return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0) {
    perror("Invalid file descriptor");
    return STATUS_ERROR;
  }

  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (!header) {
    perror("calloc");
    return STATUS_ERROR;
  }

  if (read(fd, header, sizeof(struct dbheader_t)) !=
      sizeof(struct dbheader_t)) {
    perror("read");
    free(header);
    return STATUS_ERROR;
  }

  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->magic = ntohl(header->magic);
  header->filesize = ntohl(header->filesize);

  if (header->version != 1) {
    fprintf(stderr, "improper header version");
    free(header);
    return STATUS_ERROR;
  }

  if (header->magic != HEADER_MAGIC) {
    fprintf(stderr, "improper header magic");
    free(header);
    return STATUS_ERROR;
  }
  struct stat dbstat = {0};
  fstat(fd, &dbstat);
  if (header->filesize != dbstat.st_size) {
    fprintf(stderr, "corrupted database");
    free(header);
    return STATUS_ERROR;
  }

  *headerOut = header;

  return 0;
}

int create_db_header(int fd, struct dbheader_t **headerOut) {
  (void)fd; // fd is not used in this function, but kept for consistency
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (!header) {
    perror("calloc");
    return STATUS_ERROR;
  }

  header->version = 0x1;
  header->magic = HEADER_MAGIC;
  header->count = 0;
  header->filesize = sizeof(struct dbheader_t);

  *headerOut = header;

  return STATUS_SUCCESS;
}
