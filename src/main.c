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
}

int main(int argc, char *argv[]) {
  char *filepath = NULL;
  bool newfile = false;
  int c = 0;
  int dbfd = -1;
  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  while ((c = getopt(argc, argv, "nf:")) != -1) {
    switch (c) {
    case 'n':
      newfile = true;
      break;
    case 'f':
      filepath = optarg;
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
    if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
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
      fprintf(stderr, "failed to validate database header");
      return -1;
    }
  }

  if (read_employees(dbfd, dbhdr, &employees) != STATUS_SUCCESS) {
    fprintf(stderr, "error reading employees from database");
    close(dbfd);
    return -1;
  }

  int status = output_file(dbfd, dbhdr, NULL);
  if (status != STATUS_SUCCESS) {
    fprintf(stderr, "error writing to database");
    return -1;
  }

  return 0;
}
