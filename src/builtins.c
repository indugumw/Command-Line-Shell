#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hash.h"
#include "process.h"

char *replace_newline (char *message);

char *replace_rcode (char *message);

char *replace_env (char *message);

// Given a message as input, print it to the screen followed by a
// newline ('\n'). If the message contains the two-byte escape sequence
// "\\n", print a newline '\n' instead. No other escape sequence is
// allowed. If the sequence contains a '$', it must be an environment
// variable or the return code variable ("$?"). Environment variable
// names must be wrapped in curly braces (e.g., ${PATH}).
//
// Returns 0 for success, 1 for errors (invalid escape sequence or no
// curly braces around environment variables).
int
echo (char *message)
{
  if (message == 0)
    return 1;

  char *newlined = replace_newline (message);
  char *rcoded = replace_rcode (newlined);
  free (newlined);
  char *envd = replace_env (rcoded);
  free (rcoded);

  if (envd == NULL)
    {
      printf ("Invalid env variable reference.\n");
      free (envd);
      return 1;
    }

  printf ("%s\n", envd);
  free (envd);
  return 0;
}

char *replace (char *message, char *target, char *replacement);

char *
replace_newline (char *message)
{
  return replace (message, "\\n", "\n");
}

char *
replace_rcode (char *message)
{
  // Retrieve previous return code from hash table
  char *rcode_str = hash_find ("?");

  // Replace the return code placeholder(s)
  char *result = replace (message, "$?", rcode_str);

  return result;
}

char *
replace_env (char *message)
{
  char *env_pointer = strstr (message, "${");
  char *env_end_pointer = strstr (message, "}");

  if (env_pointer == NULL || env_end_pointer == NULL)
    {
      return strdup (message);
    }

  int env_length = (env_end_pointer) - (env_pointer + 2);

  if (env_length < 1)
    // Invalid env variable reference
    return NULL;

  char *env_name = calloc (env_length + 1, sizeof (char));
  strncpy (env_name, &env_pointer[2], env_length);
  char *env = hash_find (env_name);
  free (env_name);
  if (env == NULL)
    env = "";

  char *target = calloc (env_length + 4, sizeof (char));
  strncpy (target, env_pointer, env_length + 3);

  char *result = replace (message, target, env);
  free (target);

  char *recursed = replace_env (result);
  free (result);

  return recursed;
}

char *
replace (char *message, char *target, char *replacement)
{
  char *replace_pointer = strstr (message, target);
  if (replace_pointer == NULL)
    {
      // Message doesn't contain the search string
      // Return duplicate of the original message
      return strdup (message);
    }

  // Calculate the change in size of the message after replacement
  int size_change = strlen (replacement) - strlen (target);

  // Allocate space for new string
  char *result = calloc (strlen (message) + size_change + 1, sizeof (char));

  // Add the stuff before the target if it exists
  int prefix_size = (replace_pointer - message);
  if (prefix_size > 0)
    {
      strncat (result, message, prefix_size);
    }

  // Add the replacement
  strncat (result, replacement, strlen (replacement) + 1);

  // Add the stuff after the target if it exists
  int suffix_size = strlen (message) - (prefix_size + strlen (target));
  if (suffix_size > 0)
    {
      strncat (result, &replace_pointer[strlen (target)], suffix_size);
    }

  // Recurse to cover all occurrences of the target
  char *recursed = replace (result, target, replacement);

  free (result);
  return recursed;
}

// Given a key-value pair string (e.g., "alpha=beta"), insert the mapping
// into the global hash table (hash_insert ("alpha", "beta")).
//
// Returns 0 on success, 1 for an invalid pair string (kvpair is NULL or
// there is no '=' in the string).
int export(char *kvpair)
{
  if (kvpair == NULL || strstr (kvpair, "=") == NULL)
    {
      printf ("Missing equals sign.\n");
      return 1;
    }

  char *dup = strdup (kvpair);
  char *key = strtok (kvpair, "=");
  char *value = strtok (NULL, "=");
  hash_insert (key, value);
  free (dup);
  return 0;
}

// Prints the current working directory (see getcwd()). Returns 0.
int
pwd (void)
{
  // PATH_MAX is the maximum length of a path on the current operating system
  char *cwd = calloc (PATH_MAX, sizeof (char));
  getcwd (cwd, PATH_MAX);
  printf ("%s\n", cwd);
  free (cwd);
  return 0;
}

int
cd (char *path)
{
  return chdir (path) == -1;
}

// Removes a key-value pair from the global hash table.
// Returns 0 on success, 1 if the key does not exist.
int
unset (char *key)
{
  if (hash_find (key) == NULL)
    return 1;
  hash_remove (key);
  return 0;
}

// Given a string of commands, find their location(s) in the $PATH global
// variable. If the string begins with "-a", print all locations, not just
// the first one.
//
// Returns 0 if at least one location is found, 1 if no commands were
// passed or no locations found.
int
which (char *cmdline)
{
  if (cmdline == NULL)
    return 1;

  if (is_builtin (cmdline))
    {
      printf ("%s: dukesh built-in command\n", cmdline);
      return 0;
    }

  char *path = search_path (cmdline);
  if (path == NULL)
    return 1;
  printf ("%s\n", path);
  free (path);
  return 0;
}