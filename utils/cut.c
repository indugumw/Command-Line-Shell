#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

// You may assume that lines are no longer than 1024 bytes
#define LINELEN 1024

static void usage (void);

void cut(char *line, char *delimiter, int field) {
  char *token;
  int currentField = 1;

  bool printed = false;

  token = strtok(line, delimiter);

     while (token != NULL) {
        if (currentField == field) {
            printed = true;
            printf("%s\n", token);
            break;
        }
        token = strtok(NULL, delimiter);
        currentField++;
    }

    if (printed == false) {
      printf("\n");
    }
}

int
main (int argc, char *argv[])
{
  char* delimiter = " "; 
  int field_number = 1;
  
  int ch = 0;
  char *ptr;

  while ((ch = getopt(argc, argv, "d:f:h")) != -1) 
  {
    switch (ch) 
    {
      case 'd':
        delimiter = strdup(optarg);
        break;
      case 'f':
        field_number = strtol(optarg, &ptr, 10);
        break;
      case 'h':
        usage ();
        break;
      default:
        return EXIT_FAILURE;
    }
  }

  if (field_number < 0) {
    usage ();
    return EXIT_FAILURE;
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
  while (fgets(line, LINELEN, file)) {
    line[strcspn(line, "\n")] = '\0';

    cut(line, delimiter, field_number);
  }

  if (file != stdin) {
    fclose(file);
  }

  return EXIT_SUCCESS;
}

static void
usage (void)
{
  printf ("cut, splits each line based on a delimiter\n");
  printf ("usage: cut [FLAG] FILE\n");
  printf ("FLAG can be:\n");
  printf ("  -d C     split each line based on the character C (default ' ')\n");
  printf ("  -f N     print the Nth field (1 is first, default 1)\n");
  printf ("If no FILE specified, read from STDIN\n");
}
