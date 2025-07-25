#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
  for (int i = 0; i < dbhdr->count; i++) {
    printf("Employee %d:\n", i);
    printf("\tName: %s\n", employees[i].name);
    printf("\tAddress: %s\n", employees[i].address);
    printf("\tHours: %d\n", employees[i].hours);
  }
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees,
                 char *addstring) {

  char *name = strtok(addstring, ",");
  char *addr = strtok(NULL, ",");
  char *hours_str = strtok(NULL, ",");
  strncpy(employees[dbhdr->count - 1].name, name,
          sizeof(employees[dbhdr->count - 1].name));
  strncpy(employees[dbhdr->count - 1].address, addr,
          sizeof(employees[dbhdr->count - 1].address));
  employees[dbhdr->count - 1].hours = atoi(hours_str);

  return STATUS_SUCCESS;
}

int find_employee(struct dbheader_t *dbhdr, struct employee_t *employees,
                  char *matchstring) {
  for (int i = 0; i < dbhdr->count; i++) {
    if (strcmp(employees[i].name, matchstring) == 0) {
      return i;
    }
  }

  return STATUS_ERROR;
}

int remove_employee(struct dbheader_t *dbhdr, struct employee_t *employees,
                    char *removestring) {
  int emp_index = find_employee(dbhdr, employees, removestring);
  if (emp_index < 0) {
    fprintf(stderr, "employee not found\n");
    return STATUS_ERROR;
  }

  employees[emp_index] = employees[dbhdr->count - 1];
  dbhdr->count--;

  return STATUS_SUCCESS;
}

int update_employee_hours(struct dbheader_t *dbhdr,
                          struct employee_t *employees, char *updatestring) {
  char *name = strtok(updatestring, ",");
  char *hours_str = strtok(NULL, ",");
  int emp_index = find_employee(dbhdr, employees, name);
  if (emp_index < 0) {
    fprintf(stderr, "employee not found\n");
    return STATUS_ERROR;
  }
  int new_hours = atoi(hours_str);
  if (new_hours < 0) {
    fprintf(stderr, "invalid hours, must be number\n");
    return STATUS_ERROR;
  }

  employees[emp_index].hours = new_hours;
  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr,
                   struct employee_t **employeesOut) {
  int count = dbhdr->count;
  *employeesOut = NULL;

  if (fd < 0) {
    perror("Invalid file descriptor");
    return STATUS_ERROR;
  }

  if (count == 0) {
    return STATUS_SUCCESS; // No employees to read
  }

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

  *employeesOut = employees;

  return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr,
                struct employee_t *employees) {
  if (fd < 0) {
    perror("Invalid file descriptor");
    return STATUS_ERROR;
  }
  int realcount = dbhdr->count;
  dbhdr->filesize = sizeof *dbhdr + realcount * sizeof *employees;

  dbhdr->magic = htonl(dbhdr->magic);
  dbhdr->filesize = htonl(dbhdr->filesize);
  dbhdr->count = htons(dbhdr->count);
  dbhdr->version = htons(dbhdr->version);

  lseek(fd, 0, SEEK_SET);
  write(fd, dbhdr, sizeof(struct dbheader_t));
  for (int i = 0; i < realcount; i++) {
    employees[i].hours = htonl(employees[i].hours);
    write(fd, &employees[i], sizeof(struct employee_t));
  }

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
    fprintf(stderr, "corrupted database\n");
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
