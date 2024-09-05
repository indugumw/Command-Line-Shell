#include <dirent.h>
#include <spawn.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
//#include <spawn.h>
#include <unistd.h>
#include <wait.h>

// The contents of this file are up to you, but they should be related to
// running separate processes. It is recommended that you have functions
// for:
//   - performing a $PATH lookup
//   - determining if a command is a built-in or executable
//   - running a single command in a second process
//   - running a pair of commands that are connected with a pipe

bool is_exec (char *filepath);

// Returns a path to an executable, or null. Returned pointer must be freed.
char *
search_path (char *search)
{
  // If command starts with "./", bypass path lookup
  if (strlen (search) >= 2 && search[0] == '.' && search[1] == '/')
    {
      if (is_exec (search))
        return strdup (search);
      else
        return NULL;
    }

  char *path_env = strdup (getenv ("PATH"));
  char *path = strtok (path_env, ":");
  while (path != NULL)
    {
      DIR *dir = opendir (path);
      if (dir == NULL)
        {
          path = strtok (NULL, ":");
          continue;
        }
      struct dirent *entry;
      while ((entry = readdir (dir)) != NULL)
        {
          if (strcmp (search, entry->d_name) == 0)
            {
              int path_len = strlen (entry->d_name) + strlen (path) + 2;
              char *full_path = calloc (path_len, sizeof (char));
              snprintf (full_path, path_len, "%s/%s", path, entry->d_name);
              free (path_env);
              if (!is_exec (full_path))
                {
                  free (full_path);
                  // No other files in this directory can
                  // have the same name, so this
                  // directory doesn't contain the target
                  break;
                }
              closedir (dir);
              return full_path;
            }
        }
      closedir (dir);
      path = strtok (NULL, ":");
    }
  free (path_env);
  return NULL;
}

bool
is_builtin (char *command)
{
  const char *built_ins[]
      = { "echo", "export", "pwd", "cd", "unset", "which", "quit" };
  const int n_built_ins = 7;

  for (int i = 0; i < n_built_ins; i++)
    {
      if (strcmp (command, built_ins[i]) == 0)
        {
          return true;
        }
    }
  return false;
}

bool
is_exec (char *filepath)
{
  struct stat sb;
  return stat (filepath, &sb) == 0 && sb.st_mode & S_IXUSR;
}

int
run_external (char *command, char **argv, int *prev_rcode, char **envp,
              int *in, int *out)
{
  // Get external program path
  char *path = search_path (command);

  if (path == NULL)
    {
      printf ("External program \"%s\" not found.\n", command);
      return 0;
    }

  // Launch external program and wait until it is finished
  int child = -1;
  posix_spawn_file_actions_t actions;
  posix_spawn_file_actions_init (&actions);
  if (in != NULL)
    {
      posix_spawn_file_actions_addclose (&actions, in[1]);
      posix_spawn_file_actions_adddup2 (&actions, in[0], STDIN_FILENO);
    }
  if (out != NULL)
    {
      posix_spawn_file_actions_addclose (&actions, out[0]);
      posix_spawn_file_actions_adddup2 (&actions, out[1], STDOUT_FILENO);
    }
  posix_spawn (&child, path, &actions, NULL, argv, envp);
  posix_spawn_file_actions_destroy (&actions);
  waitpid (child, prev_rcode, WUNTRACED);
  if (prev_rcode != NULL)
    {
      *prev_rcode = WEXITSTATUS (*prev_rcode);
    }
  free (path);
  return 0;
}