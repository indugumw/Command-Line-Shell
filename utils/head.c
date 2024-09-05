#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

// You may assume that lines are no longer than 1024 bytes
#define LINELEN 1024

static void usage (void);

int
main (int argc, char *argv[])
{
  int num_lines = 5;

  int ch = 0;
  char *ptr;

  while ((ch = getopt(argc, argv, "n:h")) != -1) 
  {
    switch (ch) 
    {
      case 'n':
        num_lines = strtol(optarg, &ptr, 10);
        break;
      case 'h':
        usage ();
        break;
      default:
        return EXIT_FAILURE;
    }
  }

  FILE *file = NULL;
  char *filename = argv[argc - 1];
  if (optind < argc) {
    file = fopen(filename, "r");
    if (!file) {
      return 1;
    }
  }
  else {
    file = stdin;
  }

  char line[LINELEN];
  int lines_printed = 0;

  while (fgets(line, LINELEN, file) != NULL && lines_printed < num_lines) {
    printf(line, stdout);
    lines_printed++;
  }

  if (file != stdin) {
    fclose(file);
  }
  
  return EXIT_SUCCESS;
}

static void
usage (void)
{
  printf ("head, prints the first few lines of a file\n");
  printf ("usage: head [FLAG] FILE\n");
  printf ("FLAG can be:\n");
  printf ("  -n N     show the first N lines (default 5)\n");
  printf ("If no FILE specified, read from STDIN\n");
}
