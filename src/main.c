#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
  printf("Usage: %s [-n] -f <filepath>\n", argv[0]);
  printf("\t-n: Create a new file\n");
  printf("\t-f <filepath>: Specify the file path\n");
  printf("\t-a <string>: Add an employee with the given string\n");
  printf("\t-l: List all employees\n");
  printf("\t-r <string>: Remove an employee with the given name\n");
  printf("\t-u <string>: Update hours for an employee with the given name\n");
}

int main(int argc, char *argv[]) {
  char *filepath = NULL;
  bool newfile = false;
  char *addString = NULL;
  char *removeString = NULL;
  char *updateString = NULL;
  bool list = false;
  int c = 0;
  int dbfd = -1;

  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  while ((c = getopt(argc, argv, "nf:a:lr:u:")) != -1) {
    switch (c) {
    case 'n':
      newfile = true;
      break;
    case 'f':
      filepath = optarg;
      break;
    case 'a':
      addString = optarg;
      break;
    case 'l':
      list = true;
      break;
    case 'r':
      removeString = optarg;
      break;
    case 'u':
      updateString = optarg;
      break;
    case '?':
      printf("Unknown option: -%c\n", optopt);
      break;
    default:
      return -1;
    }
  }

  if (filepath == NULL) {
    printf("filepath is a required argument.\n");
    print_usage(argv);
    return -1;
  }
  if (newfile) {
    dbfd = create_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      fprintf(stderr, "unable to create db file\n");
      return -1;
    }
    if (create_db_header(&dbhdr) == STATUS_ERROR) {
      fprintf(stderr, "unable to create db header\n");
      close(dbfd);
      return -1;
    }

  } else {
    dbfd = open_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      fprintf(stderr, "unable to open db file\n");
      return -1;
    }
    if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      fprintf(stderr, "failed to validate database header\n");
      return -1;
    }
  }

  if (read_employees(dbfd, dbhdr, &employees) != STATUS_SUCCESS) {
    fprintf(stderr, "error reading employees from database\n");
    close(dbfd);
    return -1;
  }

  if (addString != NULL) {
    dbhdr->count++;
    employees = realloc(employees, dbhdr->count * sizeof(struct employee_t));
    if (employees == NULL) {
      fprintf(stderr, "error reallocating memory for employees\n");
      close(dbfd);
      return -1;
    }
    if (add_employee(dbhdr, &employees, addString) != STATUS_SUCCESS) {
      fprintf(stderr, "error adding employee to database\n");
      close(dbfd);
      return -1;
    }
  }

  if (removeString != NULL) {
    if (remove_employee(dbhdr, &employees, removeString) != STATUS_SUCCESS) {
      fprintf(stderr, "error removing employee from database\n");
      close(dbfd);
      return -1;
    }
    int new_filesize =
        sizeof(struct dbheader_t) + (dbhdr->count * sizeof(struct employee_t));
    if (ftruncate(dbfd, new_filesize) != 0) {
      fprintf(stderr, "error truncating database file\n");
      close(dbfd);
      return -1;
    }
  }

  if (updateString != NULL) {
    if (update_employee_hours(dbhdr, &employees, updateString) !=
        STATUS_SUCCESS) {
      fprintf(stderr, "error updating employee in database\n");
      close(dbfd);
      return -1;
    }
  }

  if (list) {
    list_employees(dbhdr, employees);
  }

  int status = output_file(dbfd, dbhdr, employees);
  if (status != STATUS_SUCCESS) {
    fprintf(stderr, "error writing to database");
    return -1;
  }

  close(dbfd);

  return 0;
}
