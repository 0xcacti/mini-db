#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {}

int main(int argc, char *argv[]) {
  int c = 0;
  bool newfile = false;

  while ((c = getopt(argc, argv, "nf:") != 1)) {
    switch (c) {
    case 'f':
      break;
    case 'n':
      newfile = true;
      break;
    }
  }

  if (newfile) {
  }
}
