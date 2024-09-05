#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "../src/process.c"

static void usage (void);

void
print_env (char *envp[]);

int
main (int argc, char *argv[], char *envp[])
{
  int ch = 0;
  while ((ch = getopt (argc, argv, "h")) != -1)
    {
      switch (ch)
        {
        case 'h':
          usage ();
          return EXIT_SUCCESS;
        default:
          return EXIT_FAILURE;
        }
    }

  if (optind >= argc)
    {
      print_env (envp);
      return EXIT_SUCCESS;
    }
  
  char *path;
  char **envs = calloc (argc - optind, sizeof (char*));
  int env_index = 0;
  while (optind < argc)
    {
      if (optind == argc - 1)
        {
          path = argv[optind++];
          break;
        }
      envs[env_index++] = argv[optind++];
    }

  char **argv_new = calloc (2, sizeof (char*));
  argv_new[0] = path;

  run_external (path, argv_new, NULL, envs, NULL, NULL);

  return EXIT_SUCCESS;
}

void
print_env (char *envp[])
{
  int i = 0;
  char *env = envp[i];
  while (env != NULL)
    {
      printf ("%s\n", env);
      env = envp[++i];
    }
}

static void
usage (void)
{
  printf ("env, set environment variables and execute program\n");
  printf ("usage: env [name=value ...] PROG ARGS\n");
}

