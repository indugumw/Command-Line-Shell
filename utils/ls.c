#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>

static void usage (void);

int
main (int argc, char *argv[])
{
  bool all = false;
  bool sizes = false;
  int ch = 0;
  while ((ch = getopt (argc, argv, "ash")) != -1)
    {
      switch (ch)
        {
        case 'a':
          all = true;
          break;
        case 's':
          sizes = true;
          break;
        case 'h':
          usage ();
          return EXIT_SUCCESS;
        default:
          return EXIT_FAILURE;
        }
    }

  char *target_path = NULL;
  if (optind < argc)
      target_path = argv[optind];
  else
    {
      // PATH_MAX is the maximum length of a path on the current operating system
      target_path = calloc (PATH_MAX, sizeof (char));
      getcwd(target_path, PATH_MAX);
    }

  DIR *dir = opendir(target_path);
  if (dir == NULL)
    {
      exit(1);
    }
  
  struct dirent *entry;
  while ((entry = readdir (dir)) != NULL)
    {
      if (!all && entry->d_name[0] == '.') 
          // Skip hidden files if -a flag is not present
          continue;

      if (sizes)
        {
          int path_len = strlen (entry->d_name) + strlen (target_path) + 1;
          char *filepath = calloc (path_len + 1, sizeof (char));
          snprintf (filepath, path_len + 1, "%s/%s", target_path, entry->d_name);
          struct stat sb;
          if (stat (filepath, &sb) != 0)
            {
              continue;
            }
          printf ("%ld ", sb.st_size);
        }
      
      printf ("%s\n", entry->d_name);
    }
  closedir (dir);

  return EXIT_SUCCESS;
}

static void
usage (void)
{
  printf ("ls, list directory contents\n");
  printf ("usage: ls [FLAG ...] [DIR]\n");
  printf ("FLAG is one or more of:\n");
  printf ("  -a       list all files (even hidden ones)\n");
  printf ("  -s       list file sizes\n");
  printf ("If no DIR specified, list current directory contents\n");
}
