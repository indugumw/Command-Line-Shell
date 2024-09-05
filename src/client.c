#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dhcp.h"
#include "format.h"
#include "port_utils.h"

static bool get_args (int argc, char **argv, msg_t* request);

int
main (int argc, char **argv)
{
  msg_t request;
  memset (&request, 0, sizeof (msg_t));
  request.op = BOOTREQUEST;
  request.htype = ETH;
  request.hlen = ETH_LEN;

  if (!get_args(argc, argv, &request))
    return EXIT_FAILURE;

  dump_msg (stdout, &request, sizeof (msg_t));

  return EXIT_SUCCESS;
}

static bool
get_args (int argc, char **argv, msg_t* request)
{
  int option;
  while ((option = getopt (argc, argv, "x:t:c:m:s:r:p")) != -1)
  {
    switch (option)
    {
      case 'x':
        break;
      case 't':
        break;
      case 'c':
        break;
      case 'm':
        break;
      case 's':
        break;
      case 'r':
        break;
      case 'p':
        break;
    }
  }

  return true;
}
