//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

/* This file contains routines to inspect the routing table, discern the default
 * route, and look up the IP address of the network interface that corresponds
 * to this route. This allows software to bind to that address reliably, rather
 * than having to look up the hostname, which resolves to 127.0.0.1 on many
 * Linux distributions. */

/* This code will only work on Linux. */

#ifdef __linux__ /* [ */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <linux/if.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "os/linux/host_address.h"

static void get_default(char * if_name, int length)
{
   if(length <= 999)
   {
      FILE * routes = fopen("/proc/net/route", "r");
      if(routes)
      {
         char line[256];
         /* read header line */
         char * dummy;
         dummy = fgets(line, 256, routes);
         /* prefetch read loop */
         dummy = fgets(line, 256, routes);
         while(!feof(routes))
         {
            unsigned int address;
            char query[9];
            sprintf(query, "%%%ds %%x", length);
            sscanf(line, query, if_name, &address);
            if(!address)
               break;
            dummy = fgets(line, 256, routes);
         }
         if(feof(routes))
            if_name[0] = 0;
         fclose(routes);
      }
   }
   else
      if_name[0] = 0;
}

static unsigned int get_address(char * if_name)
{
   struct ifconf if_list;
   struct ifreq interfaces[16];
   int i, s = socket(AF_INET, SOCK_DGRAM, 0);
   if_list.ifc_len = 16 * sizeof(struct ifreq);
   if_list.ifc_req = interfaces;
   ioctl(s, SIOCGIFCONF, &if_list);
   close(s);

   for(i = 0; i != if_list.ifc_len / sizeof(struct ifreq); i++)
   {
      char * name = interfaces[i].ifr_ifrn.ifrn_name;
      unsigned int address = ((struct sockaddr_in *)&interfaces[i].ifr_ifru.ifru_addr)->sin_addr.s_addr;
      if(if_name && !strcmp(name, if_name))
         return address;
      /* if we couldn't get a default route, accept the first non-local address */
      if(!if_name && address != 0x0100007F)
         return address;
   }
   /* return 127.0.0.1 */
   return 0x0100007F;
}

static void get_hwaddr(char * if_name, char * address, int length)
{
   char * hwaddr;
   struct ifreq interface;
   int s = socket(AF_INET, SOCK_DGRAM, 0);
   strcpy(interface.ifr_name, if_name);
   ioctl(s, SIOCGIFHWADDR, &interface);
   close(s);

   hwaddr = interface.ifr_hwaddr.sa_data;
   snprintf(address, length, "%02X:%02X:%02X:%02X:%02X:%02X", hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
}

unsigned int getExternalHostAddressLinux(void)
{
   char if_name[16];
   get_default(if_name, 16);
   return get_address(if_name[0] ? if_name : NULL);
}

void getEthernetHWAddrLinux(char * address, int length)
{
   char if_name[16];
   get_default(if_name, 16);
   get_hwaddr(if_name, address, length);
}

#endif /* __linux__ ] */
