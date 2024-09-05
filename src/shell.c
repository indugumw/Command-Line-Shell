#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "hash.h"
#include "process.h"

// No command line can be more than 100 characters
#define MAXLENGTH 100

int process_command (char *buffer, int *prev_rcode);

void
shell (FILE *input)
{
  hash_init (100);
  hash_insert ("?", "0");
  hash_insert ("PATH", getenv ("PATH"));
  char buffer[MAXLENGTH];
  while (1)
    {
      // Print the cursor and get the next command entered
      printf ("$ ");
      memset (buffer, 0, sizeof (buffer));
      if (fgets (buffer, MAXLENGTH, input) == NULL)
        break;

      if (input != stdin)
        {
          printf ("%s", buffer);
          fflush (stdout);
        }

      int prev_rcode = strtol (hash_find ("?"), NULL, 10);
      int result = process_command (buffer, &prev_rcode);

      // Get length of return code as a string
      int rcode_len = snprintf (NULL, 0, "%d", prev_rcode);
      char *rcode_str = calloc (rcode_len + 1, sizeof (char));
      // Create string representation of return code
      snprintf (rcode_str, rcode_len + 1, "%d", prev_rcode);

      hash_insert ("?", rcode_str);
      free (rcode_str);

      if (!result == 0)
        break;
    }
  printf ("\n");
  hash_destroy ();
}

int split_args (char *input, char ***argv, int *argc);

char *concatenate (char **input, int len);

char **gen_envp ();

void free_envp (char **envp);

void print_pipe (int *pipefd);

int
process_command (char *buffer, int *prev_rcode)
{
  // Return value
  int result = 0;

  // Duplicate buffer for tokenization
  char *dup = strdup (buffer);

  // Remove newline from end of input
  dup[strlen (dup) - 1] = '\0';

  // Open pipe for potential child process
  int pipefd[2];
  if (pipe (pipefd) < 0)
    {
      // Failed to open pipe
      free (dup);
      return 1;
    }

  // Get piped command if present
  strtok (dup, "|");
  char *pipe_cmd = strtok (NULL, "|");
  char **pipe_argv = NULL;
  int pipe_argc = 0;
  if (pipe_cmd != NULL)
    {
      if (split_args (pipe_cmd, &pipe_argv, &pipe_argc) != 0)
        {
          // Empty pipe command
          free (dup);
          free (pipe_argv);
          return 0;
        }
    }

  // Get command/args
  char **argv = NULL;
  int argc = 0;
  if (split_args (dup, &argv, &argc) != 0)
    {
      // Empty input
      free (dup);
      free (argv);
      if (pipe_argv != NULL)
        {
          free (pipe_argv);
        }
      return 0;
    }
  char *command = argv[0];

  // Execute command
  if (strcmp (command, "quit") == 0)
    {
      result = 1;
      *prev_rcode = 0;
    }
  else if (strcmp (command, "echo") == 0)
    {
      if (argc < 2)
        *prev_rcode = echo ("");
      else
        {
          char *message = concatenate (&argv[1], argc - 1);
          *prev_rcode = echo (message);
          free (message);
        }
    }
  else if (strcmp (command, "pwd") == 0)
    {
      *prev_rcode = pwd ();
    }
  else if (strcmp (command, "cd") == 0)
    {
      if (argc < 2)
        *prev_rcode = 0;
      else
        *prev_rcode = cd (argv[1]);
    }
  else if (strcmp (command, "which") == 0)
    {
      if (argc < 2)
        *prev_rcode = 0;
      else
        *prev_rcode = which (argv[1]);
    }
  else if (strcmp (command, "export") == 0)
    {
      if (argc < 2)
        *prev_rcode = 0;
      else
        *prev_rcode = export(argv[1]);
    }
  else if (strcmp (command, "unset") == 0)
    {
      if (argc < 2)
        *prev_rcode = 0;
      else
        *prev_rcode = unset (argv[1]);
    }
  else
    {
      char **envp = gen_envp ();
      result = run_external (command, argv, prev_rcode, envp, NULL, pipefd);
      close (pipefd[1]);
      if (pipe_cmd != NULL)
        {
          int other_pipefd[2];
          if (pipe (other_pipefd) < 0)
            {
              free (dup);
              free (argv);
              free (pipe_argv);
              return 1;
            }
          result = run_external (pipe_argv[0], pipe_argv, prev_rcode, envp,
                                 pipefd, other_pipefd);
          close (pipefd[0]);
          close (other_pipefd[1]);
          print_pipe (other_pipefd);
          close (other_pipefd[0]);
        }
      else
        {
          print_pipe (pipefd);
          close (pipefd[0]);
        }
      free_envp (envp);
    }

  free (dup);
  free (argv);
  if (pipe_argv != NULL)
    {
      free (pipe_argv);
    }
  return result;
}

void
print_pipe (int *pipefd)
{
  char buffer[4096];
  size_t bytesRead;
  while ((bytesRead = read (pipefd[0], buffer, sizeof (buffer))) > 0)
    {
      write (STDOUT_FILENO, buffer, bytesRead);
      fflush (stdout);
    }
}

char **
gen_envp ()
{
  char **keys = hash_keys ();
  int size = 0;
  char *key = keys[size];
  char **envp = calloc (1, sizeof (char *));
  while (key != NULL)
    {
      envp = realloc (envp, (size + 2) * sizeof (char *));
      char *value = hash_find (key);
      size_t env_len = strlen (key) + strlen (value) + 2;
      char *env = calloc (env_len, sizeof (char));
      snprintf (env, env_len, "%s=%s", key, value);
      envp[size] = env;
      key = keys[++size];
    }
  free (keys);
  return envp;
}

void
free_envp (char **envp)
{
  int index = 0;
  char *env = envp[index];
  while (env != NULL)
    {
      free (env);
      env = envp[++index];
    }
  free (envp);
}

int
split_args (char *input, char ***argv, int *argc)
{
  // Check for empty input
  if (strlen (input) == 0)
    {
      return 1;
    }

  *argv = calloc (1, sizeof (char *));
  (*argv)[0] = strtok (input, " ");
  *argc = 1;
  char *arg = strtok (NULL, " ");
  while (arg != NULL)
    {
      *argv = realloc (*argv, sizeof (char *) * ++*argc);
      (*argv)[*argc - 1] = arg;
      arg = strtok (NULL, " ");
    }
  *argv = realloc (*argv, sizeof (char *) * (*argc + 1));
  (*argv)[*argc] = NULL;
  return 0;
}

char *
concatenate (char **input, int len)
{
  if (len < 1)
    return calloc (1, sizeof (char));

  int word_len = strlen (input[0]) + 1;
  char *result = calloc (word_len, sizeof (char));
  strncat (result, input[0], word_len);
  for (int i = 1; i < len; i++)
    {
      int word_len = strlen (input[i]) + 1;
      result = realloc (result, strlen (result) + word_len + 1);
      strncat (result, " ", 2);
      strncat (result, input[i], word_len);
    }
  return result;
}