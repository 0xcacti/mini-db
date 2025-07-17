#include <getopt.h>

int main(int argc, char *argv[]) {
  int c = 0;

  while ((c = getopt(argc, argv, "nf:") != -1)) {
    switch (c) {
    case 'f':
      break;
    case 'n':
      break;
    }
  }
}
