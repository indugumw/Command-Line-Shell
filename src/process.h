#ifndef __cs361_process__
#define __cs361_process__

// The contents of this file are up to you, but they should be related to
// running separate processes. It is recommended that you have functions
// for:
//   - performing a $PATH lookup
//   - determining if a command is a built-in or executable
//   - running a single command in a second process
//   - running a pair of commands that are connected with a pipe

char *search_path (char *search);

bool is_builtin (char *command);

int run_external (char *command, char **argv, int *prev_rcode, char **envp,
                  int *in, int *out);

#endif
