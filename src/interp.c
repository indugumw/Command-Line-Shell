#include <stdlib.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "dhcp.h"
#include "format.h"

int
main (int argc, char **argv)
{
  // Open the file as described on the command line
  char *filename = argv[1];

  int fd = open (filename, O_RDONLY);

  struct stat file_stat;
  fstat (fd, &file_stat);

  uint8_t *packet;

  // Allocate sufficient space to hold the packet
  packet = (uint8_t *)malloc (file_stat.st_size);
  memset (packet, 0, file_stat.st_size);

  // Read the packet data from the file
  read (fd, packet, file_stat.st_size);

  // Call formatting code
  dump_msg (stdout, (msg_t *)packet, file_stat.st_size);

  free (packet);
  close (fd);

  return EXIT_SUCCESS;
}

