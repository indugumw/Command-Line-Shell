#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include "dhcp.h"

void
dump_packet (uint8_t *ptr, size_t size)
{
  size_t index = 0;
  while (index < size)
    {
      fprintf (stderr, " %02" PRIx8, ptr[index++]);
      if (index % 32 == 0)
        fprintf (stderr, "\n");
      else if (index % 16 == 0)
        fprintf (stderr, "  ");
      else if (index % 8 == 0)
        fprintf (stderr, " .");
    }
  if (index % 32 != 0)
    fprintf (stderr, "\n");
  fprintf (stderr, "\n");
}

// Utility function to get hardware type
const char *get_hardware_type(uint8_t htype) 
{
  switch (htype) {
    case ETH:
      return "Ethernet (10Mb)";
    case IEEE802:
      return "IEEE 802 Networks";
    case ARCNET:
      return "ARCNET";
    case FRAME_RELAY:
      return "Frame Relay";
    case FIBRE:
      return "Fibre Channel";
    default:
      return "Unknown";
  }
}

// Print out the BOOTP fields as specified in the documentation
void interpret_bootp (FILE *output, msg_t *msg)
{
  fprintf(output, "Op Code (op) = %u [", msg->op);
  if (msg->op == BOOTREQUEST) {
    fprintf(output, "BOOTREQUEST");
  } else if (msg->op == BOOTREPLY) {
    fprintf(output, "BOOTREPLY");
  } else {
    fprintf(output, "Unknown");
  }
  fprintf(output, "]\n");

  fprintf (output, "Hardware Type (htype) = %u [%s]\n", msg->htype, get_hardware_type(msg->htype));

  fprintf (output, "Hardware Address Length (hlen) = %u\n", msg->hlen);

  fprintf (output, "Hops (hops) = %u\n", msg->hops);

  fprintf(output, "Transaction ID (xid) = %u (0x%" PRIx32 ")\n", htonl(msg->xid), htonl(msg->xid));

  int seconds = ntohs (msg->secs);
  int days = seconds / (24 * 60 * 60);
  int hours = (seconds - (days  * 24 * 60 * 60)) / (60 * 60);
  int minutes = (seconds - (days * 24 * 60 * 60) - (hours * 60 * 60)) / 60;
  fprintf (output, "Seconds (secs) = %u Days, %u:%02u:%02u\n", days, hours, minutes, seconds % 60);

  fprintf (output, "Flags (flags) = %u\n", msg->flags);

  fprintf (output, "Client IP Address (ciaddr) = %s\n", inet_ntoa(msg->ciaddr));

  fprintf (output, "Your IP Address (yiaddr) = %s\n", inet_ntoa(msg->yiaddr));

  fprintf (output, "Server IP Address (siaddr) = %s\n", inet_ntoa(msg->siaddr));

  fprintf (output, "Relay IP Address (giaddr) = %s\n", inet_ntoa(msg->giaddr));

  fprintf (output, "Client Ethernet Address (chaddr) = ");
  for (int i = 0; i < msg->hlen; ++i) {
    fprintf(output, "%02x", msg->chaddr[i]);
  }
  fprintf (output, "\n");
}
