//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

// APPLICATION INCLUDES
#include <os/HostAdapterAddress.h>
#include <os/OsSocket.h>
#include <os/OsLogger.h>

/* Get the addresses associated with all of the IP interfaces.
 * The core work is done by the SIOCGIFCONF ioctl, documented in the
 * netdevice(7) manual page, with additional hints in ioctl(2), ip(7) and inet(3).
 */
bool getAllLocalHostIps(const HostAdapterAddress* localHostAddresses[],
                        int &numAddresses)
{
   numAddresses = 0;
   UtlBoolean rc;

   // Allocate array of struct ifreq's.
   struct ifreq ifreq_array[MAX_IP_ADDRESSES];
   // Allocate struct ifconf.
   struct ifconf ifconf_structure;
   // Point ifconf to ifreq's.
   ifconf_structure.ifc_len = sizeof (ifreq_array);
   ifconf_structure.ifc_req = ifreq_array;

   // Open an arbitrary network socket on which to perform the ioctl.
   int sock = socket(PF_INET, SOCK_DGRAM, 0);
   if (sock < 0)
   {
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                    "getAllLocalHostIps unable to open socket, errno = %d '%s'",
                    errno, strerror(errno));
      rc = FALSE;
   }
   else
   {
      // Perform the SIOCGIFCONF ioctl to get the interface addresses.
      int ret = ioctl(sock, SIOCGIFCONF, (void*) &ifconf_structure);

      if (ret < 0)
      {
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                       "getAllLocalHostIps %d error performing SIOCGIFCONF, errno = %d '%s'",
                       sock, errno, strerror(errno));
         rc = FALSE;
      }
      else
      {
         rc = TRUE;
#if !defined(__MACH__) && !defined(__FreeBSD__)
         // Get the number of returned addresses from ifc_len.
         numAddresses = ifconf_structure.ifc_len / sizeof (struct ifreq);
         int j = 0;
         // Iterate through the returned addresses.
         for (int i = 0; i < numAddresses; i++)
         {
            // Get transient pointer to address in text format.
            char* s = inet_ntoa(((struct sockaddr_in&) (ifreq_array[i].ifr_addr)).sin_addr);

            // Ignore the loopback address, because opening ports on the
            // loopback interface interferes with STUN operation.
            UtlString address(s);
            if (address.compareTo("127.0.0.1") != 0 && address.compareTo("0.0.0.0") != 0)
            {
               // Put the interface name and address into a HostAdapterAddress.
               localHostAddresses[j] = new HostAdapterAddress(ifreq_array[i].ifr_name, s);
               Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                             "getAllLocalHostIps entry %d, interface '%s', address '%s'",
                             j, ifreq_array[i].ifr_name, s);
               j++;
            }
         }
         numAddresses = j;
#else
         void* ptr;
         for (ptr = ifreq_array; (intptr_t)ptr < ((intptr_t)ifreq_array) + ifconf_structure.ifc_len; )
         {
            struct ifreq* ifr = (struct ifreq*) ptr;

            int len = sizeof(struct sockaddr);
            if (ifr->ifr_addr.sa_len > sizeof(struct sockaddr))
               len = ifr->ifr_addr.sa_len;

            ptr = (void *) ((intptr_t)ptr + sizeof(ifr->ifr_name) + len);

            // The body of this if statement should mirror the for loop above...
            if (ifr->ifr_addr.sa_family == AF_INET)
            {
               char* s = inet_ntoa(((struct sockaddr_in*) &ifr->ifr_addr)->sin_addr);
               UtlString address(s);
               if (address.compareTo("127.0.0.1") != 0 && address.compareTo("0.0.0.0") != 0)
               {
                  localHostAddresses[numAddresses] = new HostAdapterAddress(ifr->ifr_name, s);
                  Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                                "getAllLocalHostIps entry %d, interface '%s', address '%s'",
                                numAddresses, ifr->ifr_name, s);
                  numAddresses++;
               }
            }
         }
#endif
      }
      close(sock);
   }
   return rc;
}

bool getContactAdapterName(char* szAdapter, const char* szIp)
{
   bool found = false;

   UtlString ipAddress(szIp);

   int numAddresses = 0;
   const HostAdapterAddress* adapterAddresses[MAX_IP_ADDRESSES];
   getAllLocalHostIps(adapterAddresses, numAddresses);

   for (int i = 0; i < numAddresses; i++)
   {
      if (ipAddress.compareTo(adapterAddresses[i]->mAddress.data()) == 0)
      {
         strcpy(szAdapter, adapterAddresses[i]->mAdapter.data());
         Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                       "getContactAdapterName found name %s for ip %s",
                       szAdapter, szIp);
         found = true;
      }
      delete adapterAddresses[i];
   }

   if (found)
   {
      return true;
   }
   else
   {
      return false;
   }
}
