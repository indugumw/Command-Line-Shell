#include <stdio.h>

#include <arpa/inet.h>
#include "dhcp.h"
#include "format.h"
#include <inttypes.h>
#include <string.h>

#include "dhcp.h"
#include "format.h"

void
dump_msg (FILE *output, msg_t *msg, size_t size)
{
  fprintf (output, "------------------------------------------------------\n");
  fprintf (output, "BOOTP Options\n");
  fprintf (output, "------------------------------------------------------\n");

  // Print BOOTP Options
  interpret_bootp (output, msg);

  // Check if DHCP options exist
  if (msg->options[0] != 0)
    {
      fprintf (output,
               "------------------------------------------------------\n");
      fprintf (output, "DHCP Options\n");
      fprintf (output,
               "------------------------------------------------------\n");

      fprintf (output, "Magic Cookie = [OK]\n");

      bool time = false;

      // Iterate through DHCP options
      for (int i = 0; i < sizeof (msg->options); i++)
        {
          if (msg->options[i] == 53)
            {
              switch (msg->options[i] = msg->options[i + 2])
                {
                case DHCPDISCOVER:
                  fprintf (output, "Message Type = DHCP Discover\n");
                  break;
                case DHCPOFFER:
                  fprintf (output, "Message Type = DHCP Offer\n");
                  time = true;
                  break;
                case DHCPREQUEST:
                  fprintf (output, "Message Type = DHCP Request\n");
                  break;
                case DHCPDECLINE:
                  fprintf (output, "Message Type = DHCP Decline\n");
                  break;
                case DHCPACK:
                  fprintf (output, "Message Type = DHCP ACK\n");
                  time = true;
                  break;
                case DHCPNAK:
                  fprintf (output, "Message Type = DHCP NAK\n");
                  break;
                case DHCPRELEASE:
                  fprintf (output, "Message Type = DHCP Release\n");
                  break;
                default:
                  fprintf (output, "Message Type = Unknown\n");
                }
            }
            
          // Check for IP Address Lease Time option
          if (msg->options[i] == 51 && time)
            {
              int seconds = ntohl (*(uint32_t *)&msg->options[i + 2]);
              int days = seconds / (24 * 60 * 60);
              int hours = (seconds - (days * 24 * 60 * 60)) / (60*60);
              int minutes = (seconds - (days * 24 * 60 * 60) - (hours * 60 * 60)) / 60;
              fprintf (output, "IP Address Lease Time = %u Days, %01u:%02u:%02u\n", days, hours, minutes, seconds % 60);
            }

          // Check for Server Identifier option
          if (msg->options[i] == 54)
            {
              uint32_t server = ntohl (*(uint32_t *)&msg->options[i + 2]);
              fprintf (output, "Server Identifier = %u.%u.%u.%u\n", (server >> 24) & 0xFF,
                 (server >> 16) & 0xFF, (server >> 8) & 0xFF, server & 0xFF);
            }

          // Check for Requested IP option
          if (msg->options[i] == 50 && msg->options[i+1] == 4)
            {
              uint32_t ip =  *(uint32_t *)&msg->options[i + 2];
              fprintf (output, "Request = %s\n", inet_ntoa (*(struct in_addr *)&ip));
        }
    }
}
