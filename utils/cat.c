#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

static void usage (void);

int
main (int argc, char *argv[])
{
  int ch = 0;

  while ((ch = getopt(argc, argv, "h")) != -1) 
  {
    switch (ch) 
    {
      case 'h':
        usage ();
        break;
      default:
        return EXIT_FAILURE;
    }
  }

  FILE *file = NULL;
  char *filename = argv[argc - 1];
  if (filename) {
    file = fopen(filename, "r");
    if (!file) {
      return 1;
    }
  }
  else {
    file = stdin;
  }

  char character;
    while ((character = fgetc(file)) != EOF)
    {
        printf("%c", character);
    }

    fclose(file);

  return EXIT_SUCCESS;
}

static void
usage (void)
{
  printf ("cat, print the contents of a file\n");
  printf ("usage: cat FILE\n");
}
