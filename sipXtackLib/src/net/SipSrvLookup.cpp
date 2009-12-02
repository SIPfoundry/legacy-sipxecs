//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// Rewritten based on DomainSearch by Christian Zahl, and SipSrvLookup
// by Henning Schulzrinne.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
#       include "resparse/wnt/sysdep.h"
#       include <resparse/wnt/netinet/in.h>
#       include <resparse/wnt/arpa/nameser.h>
#       include <resparse/wnt/resolv/resolv.h>
#       include <winsock.h>
extern "C" {
#       include "resparse/wnt/inet_aton.h"
}
#elif defined(_VXWORKS)
#       include <netdb.h>
#       include <netinet/in.h>
/* Use local lnameser.h for info missing from VxWorks version --GAT */
/* lnameser.h is a subset of resparse/wnt/arpa/nameser.h                */
#       include <resolv/nameser.h>
#       include <resparse/vxw/arpa/lnameser.h>
/* Use local lresolv.h for info missing from VxWorks version --GAT */
/* lresolv.h is a subset of resparse/wnt/resolv/resolv.h               */
#       include <resolv/resolv.h>
#       include <resparse/vxw/resolv/lresolv.h>
/* #include <sys/socket.h> used sockLib.h instead --GAT */
#       include <sockLib.h>
#       include <resolvLib.h>
#       include <resparse/vxw/hd_string.h>
#elif defined(__pingtel_on_posix__)
#       include <arpa/inet.h>
#       include <netinet/in.h>
#       include <sys/socket.h>
#       include <resolv.h>
#       include <netdb.h>
#else
#       error Unsupported target platform.
#endif

#include <sys/types.h>

// Standard C includes.
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// Application includes.
#include "os/OsSocket.h"
#include "os/OsLock.h"
#include "net/SipSrvLookup.h"
#include "os/OsSysLog.h"
#include "resparse/rr.h"

// The space allocated for returns from res_query.
#define DNS_RESPONSE_SIZE 4096

// The initial value of OptionCodeCNAMELImit.
#define DEFAULT_CNAME_LIMIT 5

// Forward references

// All of these functions are made forward references here rather than
// being protected methods in SipSrvLookup.h because some of them
// require #include "resparse/rr.h", which ultimately includes
// /usr/include/arpa/nameser_compat.h, which #defines STATUS, which is
// used in other places in our code for other purposes.

/**
 * @name Server List
 *
 * These methods maintain a list of servers and their properties that have
 * been found so far during the current search.
 * The list is a malloc'ed array of pointers to server_t's, and is
 * represented by a pointer to the array, a count of the allocated length
 * of the array, and a count of the number of entries in the array that
 * are used.
 */
///@{

/// Initialize the variables pointing to the list of servers found thus far.
static void server_list_initialize(server_t*& list,
                                   int& list_length_allocated,
                                   int& list_length_used);

///@}

/// Insert records into the list for a server address.
static void server_insert_addr(
   /// List control variables.
   server_t*& list,
   int& list_length_allocated,
   int& list_length_used,
   /// Components of the server_t.
   const char *host,
   ///< (copied)
   OsSocket::IpProtocolSocketType type,
   struct sockaddr_in sin,
   unsigned int priority,
   unsigned int weight);
/**<
 * If type is UNKNOWN (meaning no higher-level process has specified
 * the transport to this server, server_insert_addr may insert two
 * records, one for UDP and one for TCP.
 */

/**
 * Add server_t to the end of a list of server addresses.
 * No longer calculates the sorting score -- that is now done in
 * SipSrvLookup::servers.
 */
static void server_insert(
   /// List control variables.
   server_t*& list,
   int& list_length_allocated,
   int& list_length_used,
   /// Components of the server_t.
   const char *host,
   ///< (copied)
   OsSocket::IpProtocolSocketType type,
   struct sockaddr_in sin,
   unsigned int priority,
   unsigned int weight);

/**
 * Look up SRV records for a domain name, and from them find server
 * addresses to insert into the list of servers.
 */
static void lookup_SRV(server_t*& list,
                       int& list_length_allocated,
                       int& list_length_used,
                       const char *domain,
                       ///< domain name
                       const char *service,
                       ///< "sip" or "sips"
                       const char *proto_string,
                       ///< protocol string for DNS lookup
                       OsSocket::IpProtocolSocketType proto_code
                       ///< protocol code for result list
   );

/**
 * Look up A records for a domain name, and insert them into the list
 * of servers.
 */
static void lookup_A(server_t*& list,
                     int& list_length_allocated,
                     int& list_length_used,
                     const char *domain,
                     ///< domain name
                     OsSocket::IpProtocolSocketType proto_code,
                     /**< protocol code for result list
                      *   UNKNOWN means both UDP and TCP are acceptable
                      *   SSL must be set explicitly. */
                     res_response* in_response,
                     ///< current DNS response, or NULL
                     int port,
                     ///< port
                     unsigned int priority,
                     ///< priority
                     unsigned int weight
                     ///< weight
   );
/**<
 * If in_response is non-NULL, use it as an initial source of A records.
 *
 * @returns TRUE if one or more addresses were added to the list.
 */

/**
 * Search for an RR with 'name' and 'type' in the answer and additional
 * sections of a DNS response.
 *
 * @return pointer to rdata structure for the first RR founr, or NULL.
 */
static union u_rdata* look_for(res_response* response,
                               ///< response to look in
                               const char* name,
                               ///< domain name
                               int type
                               ///< RR type
   );

// Functions to compare two server entries.
// 3261 says to prefer TCP for large messages

// sort server list so TCP forks will be tried last (UDP preferred)
static int server_compare_prefer_udp(const void* a, const void* b);

// sort server list so UDP forks will be tried last (TCP preferred)
static int server_compare_prefer_tcp(const void* a, const void* b);

static int server_compare(const void* a, const void* b,
                          OsSocket::IpProtocolSocketType leastPreferredTransport);
/**<
 * Compares two server_t's which represent two servers.
 * Used by qsort to sort the list of server entries into preference
 * order.  The sort rules are that the first (smallest, and in
 * ordinary use, most preferred) element is:
 * # Lowest priority
 * # Highest weighting score
 * # Other transport types (UDP) are preferred over TCP for small messages
 * # Other transport types (TCP) are preferred over UDP for large messages
 *
 * @returns Integer comparison result as needed by qsort.
 */

// Sort servers into canonical order before calculating scores.
static int server_compare_presort(const void* a, const void* b);

static void sort_answers(res_response* response);

static int rr_compare(const void* a, const void* b);

/**
 * The array of option values.
 *
 * Set the initial values.
 */
int SipSrvLookup::options[OptionCodeLast+1] = {
   0,                           // OptionCodeNone
   0,                           // OptionCodeFirst
   0,                           // OptionCodeIgnoreSRV
   0,                           // OptionCodeIgnoreNAPTR
   0,                           // OptionCodeSortAnswers
   0,                           // OptionCodePrintAnswers
   DEFAULT_CNAME_LIMIT,         // OptionCodeCNAMELimit
   0,                           // OptionCodeNoDefaultTCP
   0,                           // OptionCodeSortServers
   0,                           // OptionCodeLast
};

/// Sets the timeout parameter for DNS SRV queries. Default is 3
int SipSrvLookup::mTimeout = 3;

/// Sets the number of retries for DNS SRV queries. Default is 2
int SipSrvLookup::mRetries = 2;

/// Sets the IP address of the nameserver. If NULL we use the default Nameservers
UtlString SipSrvLookup::mNameserverIP = "";

/// Sets the port number  of the nameserver. If NULL we use the default port number 53
int SipSrvLookup::mNameserverPort = 0;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/// Get the list of server entries for SIP domain name 'domain'.
server_t* SipSrvLookup::servers(const char* domain,
                                ///< SIP domain name or host name
                                const char* service,
                                ///< "sip" or "sips"
                                OsSocket::IpProtocolSocketType socketType,
                                ///< types of transport
                                int port,
                                ///< port number from URI, or PORT_NONE
                                OsSocket::IpProtocolSocketType preferredTransport
                                ///< preferred transport protocol
   )
{
   server_t* serverList;
   int list_length_allocated;
   int list_length_used = 0;
   struct sockaddr_in in;

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSrvLookup::servers domain = '%s', service = '%s', "
                 "socketType = %s, port = %d",
                 domain, service, OsSocket::ipProtocolString(socketType), port);

   // Initialize the list of servers.
   server_list_initialize(serverList, list_length_allocated, list_length_used);

   // Seize the lock.
   OsLock lock(sMutex);

   // Case 0: Eliminate contradictory combinations of service and type.

   // While a sip: URI can be used with a socketType of SSL_SOCKET
   // (e.g., <sip:foo@example.com;transport=tls>), a sips: URI must
   // be used with TLS.
   if ((strcmp(service, "sips") == 0 &&
        (socketType == OsSocket::TCP || socketType == OsSocket::UDP)))
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "SipSrvLookup::servers Incompatible service '%s' and "
                    "socketType %d",
                    service, socketType);
      /* Add no elements to the list. */
   }
   // Case 1: Domain name is a numeric IP address.
   else if (inet_aton(domain, &in.sin_addr))
   {
      in.sin_family = AF_INET;
      // Set up the port number.
      // If port was specified in the URI, that is the port to use.
      // if not specified, use 5061 for sips service; use 5060 for anything else.
      in.sin_port = htons(portIsValid(port) ? port :
                          ((strcmp(service, "sips") == 0) || (socketType == OsSocket::SSL_SOCKET)) ?
                          5061 : 5060);
      // If sips service, make sure transport is set correctly.
      if (socketType == OsSocket::UNKNOWN &&
          strcmp(service, "sips") == 0)
      {
         socketType = OsSocket::SSL_SOCKET;
      }
      server_insert_addr(serverList, list_length_allocated, list_length_used,
                         domain, socketType, in, 0, 0);
   }
   else
   {
      // Free unused server_t instances, pointer must always be reloaded in this path
      delete[] serverList;

      SipSrvLookupThread ** myQueryThreads = SipSrvLookupThread::getLookupThreads();

      // Initialize the SRV lookup thread args, and the A Record lookup thread args.
      // They are initialized separately as the A Records are only needed if SRV
      // records don't exist for the domain. (Or if a port name is included in the
      // domain).
      server_t * srv_record_list;
      server_list_initialize(srv_record_list, list_length_allocated, list_length_used);
      SrvThreadArgs srvLookupArgs(domain, service, srv_record_list, list_length_allocated,
                                  list_length_used, port, socketType);

      server_t * a_record_list;
      server_list_initialize(a_record_list, list_length_allocated, list_length_used);
      SrvThreadArgs aRecordLookupArgs(domain, service, a_record_list, list_length_allocated,
                                      list_length_used, port, socketType);

      myQueryThreads[SipSrvLookupThread::A_RECORD]->postMessage(aRecordLookupArgs);

      // Case 2: SRV records exist for this domain.
      // (Only used if no port is specified in the URI.)
      if (port <= 0 && !options[OptionCodeIgnoreSRV])
      {
         // If UDP transport is acceptable.
         if ((socketType == OsSocket::UNKNOWN ||
              socketType == OsSocket::UDP) &&
             strcmp(service, "sips") != 0)
         {
             myQueryThreads[SipSrvLookupThread::SRV_UDP]->postMessage(srvLookupArgs);
         }
         // If TCP transport is acceptable.
         if ((socketType == OsSocket::UNKNOWN ||
              socketType == OsSocket::TCP) &&
             strcmp(service, "sips") != 0)
         {
             myQueryThreads[SipSrvLookupThread::SRV_TCP]->postMessage(srvLookupArgs);
         }

         // If TLS transport is acceptable.
         if (socketType == OsSocket::UNKNOWN ||
              socketType == OsSocket::SSL_SOCKET)
         {
            myQueryThreads[SipSrvLookupThread::SRV_TLS]->postMessage(srvLookupArgs);
         }

         // Wait for each of the executed queries to finish
         // If UDP SRV query was carried out
         if ((socketType == OsSocket::UNKNOWN ||
              socketType == OsSocket::UDP) &&
              strcmp(service, "sips") != 0)
         {
             myQueryThreads[SipSrvLookupThread::SRV_UDP]->isDone();
         }
         // If TCP SRV query was carried out
         if ((socketType == OsSocket::UNKNOWN ||
              socketType == OsSocket::TCP) &&
              strcmp(service, "sips") != 0)
         {
            myQueryThreads[SipSrvLookupThread::SRV_TCP]->isDone();
         }
         // If TLS SRV query was carried out
         if (socketType == OsSocket::UNKNOWN ||
              socketType == OsSocket::SSL_SOCKET)
         {
            myQueryThreads[SipSrvLookupThread::SRV_TLS]->isDone();
         }

      }
      // Finally wait for the A Record Query to finish as well
      myQueryThreads[SipSrvLookupThread::A_RECORD]->isDone();

      // Check if there is a need for A records.
      // (Only used for non-numeric addresses for which SRV lookup did not
      // produce any addresses.  This includes if an explicit port was given.)
      if (*(srvLookupArgs.list_length_used) < 1)
      {
         // No SRV query results. Discard the SRV Lookup lists, continue with
         // and  return the A Record list back to the caller.
         delete[] srvLookupArgs.list;
         serverList = aRecordLookupArgs.list;
         list_length_used = *(aRecordLookupArgs.list_length_used);
         list_length_allocated = *(aRecordLookupArgs.list_length_allocated);
      }
      else
      {
         // We got SRV query results. Discard the A Record Lookup lists,
         // continue with and return the SRV list back to the caller.
         delete[] aRecordLookupArgs.list;
         serverList = srvLookupArgs.list;
         list_length_used = *(srvLookupArgs.list_length_used);
         list_length_allocated = *(srvLookupArgs.list_length_allocated);

      }
   }

   // If testing the code, sort the list of servers into a canonical order,
   // so the pseudo-random scores we calculate for them are deterministic.
   if (options[OptionCodeSortServers])
   {
      qsort(serverList, list_length_used, sizeof (server_t), server_compare_presort);
   }

   // Apply the scores to the list of servers.
   // Why we construct it this way is described in
   // sipXtackLib/doc/developer/scores/README.
   for (int j = 0; j < list_length_used; j++)
   {
      if (serverList[j].weight == 0)
      {
         // If weight is 0, set score to infinity.
         serverList[j].score = 1000;
      }
      else
      {
         int i = rand();
         // If random number is 0, change it to 1, so log() doesn't have a problem.
         if (i == 0)
         {
            i = 1;
         }
         serverList[j].score = - log(((float) i) / RAND_MAX) / serverList[j].weight;
      }
   }

   // Sort the list of servers found by priority and score.
   if (preferredTransport == OsSocket::UDP) // list order is UDP before TCP
   {
       qsort(serverList, list_length_used, sizeof (server_t),
             server_compare_prefer_udp);
   }
   else // list order puts TCP before UDP (large requests need this)
   {
       qsort(serverList, list_length_used, sizeof (server_t),
             server_compare_prefer_tcp);
   }

   // Add ending empty element to list (after sorting the real entries).
   memset(&in, 0, sizeof(in)) ;
   server_insert(serverList, list_length_allocated, list_length_used,
                 NULL, OsSocket::UNKNOWN, in, 0, 0);

   // Return the list of servers.
   if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
   {
      // Debugging print of list of servers.
      for (int j = 0; j < list_length_used; j++)
      {
         if (serverList[j].isValidServerT())
         {
            UtlString host;
            serverList[j].getHostNameFromServerT(host);
            UtlString ip_addr;
            serverList[j].getIpAddressFromServerT(ip_addr);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipSrvLookup::servers host = '%s', IP addr = '%s', "
                          "port = %d, weight = %u, score = %f, "
                          "priority = %u, proto = %s",
                          host.data(), ip_addr.data(),
                          serverList[j].getPortFromServerT(),
                          serverList[j].getWeightFromServerT(),
                          serverList[j].getScoreFromServerT(),
                          serverList[j].getPriorityFromServerT(),
                          OsSocket::ipProtocolString(serverList[j].getProtocolFromServerT())
                          );
         }
      }
   }
   return serverList;
}

/// Set an option value.
void SipSrvLookup::setOption(OptionCode option, int value)
{
   // Seize the lock, to ensure atomic effect.
   OsLock lock(sMutex);

   options[option] = value;
}

//! Sets the DNS SRV times.  Defaults: timeout=5, retries=4
void SipSrvLookup::setDnsSrvTimeouts(int initialTimeoutInSecs, int retries)
{
   if (initialTimeoutInSecs > 0)
   {
      mTimeout = initialTimeoutInSecs;
   }

   if (retries > 0)
   {
      mRetries = retries;
   }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/*
 * Lock to protect the resolver routines, which cannot tolerate multithreaded
 * use.
 */
OsMutex SipSrvLookup::sMutex(OsMutex::Q_PRIORITY |
                             OsMutex::DELETE_SAFE |
                             OsMutex::INVERSION_SAFE);

// Initialize the variables pointing to the list of servers found thus far.
void server_list_initialize(server_t*& list,
                            int& list_length_allocated,
                            int& list_length_used)
{
   list_length_allocated = 2;
   list = new server_t[list_length_allocated];
   list_length_used = 0;
}

// Add server_t to the end of a list of server addresses.
void server_insert_addr(server_t*& list,
                        int& list_length_allocated,
                        int& list_length_used,
                        const char* host,
                        OsSocket::IpProtocolSocketType type,
                        struct sockaddr_in sin,
                        unsigned int priority,
                        unsigned int weight)
{
   if (type != OsSocket::UNKNOWN)
   {
      // If the transport is specified, just insert the record.
      server_insert(list, list_length_allocated, list_length_used,
                    host, type, sin, priority, weight);
   }
   else
   {
      // If the transport is not specified, insert a UDP record.
      server_insert(list, list_length_allocated, list_length_used,
                    host, OsSocket::UDP, sin, priority, weight);
      // If specified, insert a TCP record.
      if (!SipSrvLookup::getOption(SipSrvLookup::OptionCodeNoDefaultTCP))
      {
         server_insert(list, list_length_allocated, list_length_used,
                       host, OsSocket::TCP, sin, priority, weight);
      }
   }
}

// Add server_t to the end of a list of server addresses.
void server_insert(server_t*& list,
                   int& list_length_allocated,
                   int& list_length_used,
                   const char* host,
                   OsSocket::IpProtocolSocketType type,
                   struct sockaddr_in sin,
                   unsigned int priority,
                   unsigned int weight)
{
   // Make sure there is room in the list.
   if (list_length_used == list_length_allocated)
   {
      // Allocate the new list.
      int new_length = 2 * list_length_allocated;
      server_t* new_list = new server_t[new_length];
      // Copy all the elements binarily, to avoid the overhead of
      // duplicating all the host strings.
      memcpy((char*) new_list, (char*) list,
            list_length_used * sizeof (server_t));
      // Erase the host pointers in the old list.
      for (int i = 0; i < list_length_used; i++)
      {
         list[i].host = NULL;
      }
      // Free the old list.
      delete[] list;
      // Replace the old list with the new one.
      list = new_list;
      list_length_allocated = new_length;
   }

   // Copy the element into the list.
   list[list_length_used].host =
      host != NULL ? strdup(host) : NULL;
   list[list_length_used].type = type;
   list[list_length_used].sin = sin;
   list[list_length_used].priority = priority;
   list[list_length_used].weight = weight;
   // Score will be calculated later.
   list[list_length_used].score = 0;

   // Increment the count of elements in the list.
   list_length_used++;
}

/*
 * Look up SRV records for a domain name, and from them find server
 * addresses to insert into the list of servers.
 */
void lookup_SRV(server_t*& list,
                int& list_length_allocated,
                int& list_length_used,
                const char* domain,
                ///< domain name
                const char* service,
                ///< "sip" or "sips"
                const char* proto_string,
                ///< protocol string for DNS lookup
                OsSocket::IpProtocolSocketType proto_code
                ///< protocol code for result list
   )
{
   // To hold the return of res_query_and_parse.
   res_response* response;
   const char* canonical_name;

   // Construct buffer to hold the key string for the lookup:
   //    _service._protocol.domain
   // 5 bytes suffices for the added components and the ending NUL.
   char* lookup_name = (char*) malloc(strlen(service) + strlen(proto_string) +
                                      strlen(domain) + 5);

   // Construct the domain name to search on.
   sprintf(lookup_name, "_%s._%s.%s", service, proto_string, domain);

   // Make the query and parse the response.
   SipSrvLookup::res_query_and_parse(lookup_name, T_SRV, NULL, canonical_name,
                                     response);
   if (response != NULL)
   {
       unsigned int i;
      // For each answer that is an SRV record for this domain name.

      // Search the answer list of RRs.
      for (i = 0; i < response->header.ancount; i++)
      {
         if (response->answer[i]->rclass == C_IN &&
             response->answer[i]->type == T_SRV &&
             // Note we look for the canonical name now.
             strcasecmp(canonical_name, response->answer[i]->name) == 0)
         {
            // Call lookup_A to get the A records for the target host
            // name.  Give it the pointer to our current response,
            // because it might have the A records.  If not, lookup_A
            // will do a DNS lookup to get them.
            lookup_A(list, list_length_allocated, list_length_used,
                     response->answer[i]->rdata.srv.target, proto_code,
                     response,
                     response->answer[i]->rdata.srv.port,
                     response->answer[i]->rdata.srv.priority,
                     response->answer[i]->rdata.srv.weight);
         }
      }
      // Search the additional list of RRs.
      for (i = 0; i < response->header.arcount; i++)
      {
         if (response->additional[i]->rclass == C_IN &&
             response->additional[i]->type == T_SRV &&
             // Note we look for the canonical name now.
             strcasecmp(canonical_name, response->additional[i]->name) == 0)
         {
            // Call lookup_A to get the A records for the target host
            // name.  Give it the pointer to our current response,
            // because it might have the A records.  If not, lookup_A
            // will do a DNS lookup to get them.
            lookup_A(list, list_length_allocated, list_length_used,
                     response->additional[i]->rdata.srv.target, proto_code,
                     response,
                     response->additional[i]->rdata.srv.port,
                     response->additional[i]->rdata.srv.priority,
                     response->additional[i]->rdata.srv.weight);
         }
      }
   }

   // Free the result of res_parse.
   if (response != NULL)
   {
      res_free(response);
   }
   if (canonical_name != NULL && canonical_name != lookup_name)
   {
      free((void*) canonical_name);
   }
   free((void*) lookup_name);
}

/*
 * Look up A records for a domain name, and insert them into the list
 * of servers.
 */
void lookup_A(server_t*& list,
              int& list_length_allocated,
              int& list_length_used,
              const char* domain,
              ///< domain name
              OsSocket::IpProtocolSocketType proto_code,
              ///< protocol code for result list
              res_response* in_response,
              ///< current DNS response, or NULL
              int port,
              ///< port
              unsigned int priority,
              ///< priority
              unsigned int weight
              ///< weight
   )
{
   // To hold the return of res_query_and_parse.
   res_response* response;
   const char* canonical_name;

   // Make the query and parse the response.
   SipSrvLookup::res_query_and_parse(domain, T_A, in_response, canonical_name,
                                     response);

   OsLock lock(SipSrvLookupThread::slookupThreadMutex);

   // Search the list of RRs.
   // For each answer that is an A record for this domain name.
   if (response != NULL)
   {
       unsigned int i;
      // Search the answer list.
      for (i = 0; i < response->header.ancount; i++)
      {
         if (response->answer[i]->rclass == C_IN &&
             response->answer[i]->type == T_A &&
             // Note we look for the canonical name now.
             strcasecmp(canonical_name, response->answer[i]->name) == 0)
         {
            // An A record has been found.
            // Assemble the needed information and add it to the server list.
            struct sockaddr_in sin;
	    memset(&sin, 0, sizeof(struct sockaddr_in));
            sin.sin_addr = response->answer[i]->rdata.address;
            sin.sin_family = AF_INET;
            sin.sin_port = htons(port);
            server_insert_addr(list, list_length_allocated,
                               list_length_used,
                               (const char*) domain,
                               proto_code, sin, priority, weight);
         }
      }
      // Search the additional list.
      for (i = 0; i < response->header.arcount; i++)
      {
         if (response->additional[i]->rclass == C_IN &&
             response->additional[i]->type == T_A &&
             // Note we look for the canonical name now.
             strcasecmp(canonical_name, response->additional[i]->name) == 0)
         {
            // An A record has been found.
            // Assemble the needed information and add it to the server list.
            struct sockaddr_in sin;
	    memset(&sin, 0, sizeof(struct sockaddr_in));
            sin.sin_addr = response->additional[i]->rdata.address;
            sin.sin_family = AF_INET;
            sin.sin_port = htons(port);
            server_insert_addr(list, list_length_allocated,
                               list_length_used,
                               (const char*) domain,
                               proto_code, sin, priority, weight);
         }
      }
   }

   // Free the result of res_parse if necessary.
   if (response != NULL && response != in_response)
   {
      res_free(response);
   }
   if (canonical_name != NULL && canonical_name != domain)
   {
      free((void*) canonical_name);
   }
}

// Perform a DNS query and parse the results.  Follows CNAME records.
void SipSrvLookup::res_query_and_parse(const char* in_name,
                                       int type,
                                       res_response* in_response,
                                       const char*& out_name,
                                       res_response*& out_response
   )
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSrvLookup::res_query_and_parse in_name = '%s', "
                 "type = %d (%s)",
                 in_name,type,
                 type == T_CNAME ? "CNAME" :
                 type == T_SRV ? "SRV" :
                 type == T_A ? "A" :
                 type == T_NAPTR ? "NAPTR" :
                 "unknown");

   // The number of CNAMEs we have followed.
   int cname_count = 0;
   // The response currently being examined.
   res_response* response = in_response;
   // The name currently being examined.
   const char* name = in_name;
   // TRUE if 'response' was a lookup for 'name' and 'type'.
   UtlBoolean response_for_this_name = FALSE;
   // Buffer into which to read DNS replies.
   char answer[DNS_RESPONSE_SIZE];
   union u_rdata* p;

   // Loop until we find a reason to exit.  Each turn around the loop does
   // another DNS lookup.
   while (1)
   {
      // While response != NULL and there is a CNAME record for name
      // in response.
      while (response != NULL &&
             (p = look_for(response, name, T_CNAME)) != NULL)
      {
         cname_count++;
         if (cname_count > SipSrvLookup::getOption(SipSrvLookup::OptionCodeCNAMELimit))
         {
            break;
         }
         // If necessary, free the current 'name'.
         if (name != in_name)
         {
            free((void*) name);
         }
         // Copy the canonical name from the CNAME record into 'name', so
         // we can still use it after freeing 'response'.
         name = strdup(p->string);
         // Remember that we are now looking for a name that was not the one
         // that we searched for to produce this response.  Hence, if we don't
         // find any RRs for it, that is not authoritative and we have to do
         // another DNS query.
         response_for_this_name = FALSE;
         // Go back and check whether the result name of the CNAME is listed
         // in this response.
      }
      // This response does not contain a CNAME for 'name'.  So it is either
      // a final response that gives us the RRs we are looking for, or
      // we need to do a lookup on 'name'.

      // Check whether the response was for this name, or contains records
      // of the type we are looking for.  If either, then any records we
      // are looking for are in this response, so we can return.
      if (response_for_this_name ||
          (response != NULL && look_for(response, name, type) != NULL))
      {
         break;
      }

      // We must do another lookup.
      // Start by freeing 'response' if we need to.
      if (response != in_response)
      {
         res_free(response);
      }
      response = NULL;
      // Now, 'response' will be from a query for 'name'.
      response_for_this_name = TRUE;
      // Debugging print.
      if (SipSrvLookup::getOption(SipSrvLookup::OptionCodePrintAnswers))
      {
         printf("res_nquery(\"%s\", class = %d, type = %d)\n",
                name, C_IN, type);
      }

      // Initialize the res state struct and set the timeout to
      // 3 secs and retries to 2
      struct __res_state res;
      res_ninit(&res);
      res.retrans = mTimeout;
      res.retry = mRetries;

      if (!mNameserverIP.isNull())
      {
          res.nscount = 1;
          inet_aton(mNameserverIP.data(), &res.nsaddr_list[0].sin_addr);

          if (mNameserverPort > 1)
          {
             res.nsaddr_list[0].sin_port = htons(mNameserverPort);
          }
      }

      // Use res_nquery, not res_search or res_query, so defaulting rules are not
      // applied to the domain, and so that the query is thread-safe.
      int r = res_nquery(&res, name, C_IN, type,
                         (unsigned char*) answer, sizeof (answer));
      // Done with res state struct, so cleanup.
      // Must close once and only once per res_ninit, after res_nquery.
      res_nclose(&res);

      if (r == -1)
      {
         // res_query failed, return.
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "DNS query for name '%s', "
                       "type = %d (%s): returned error",
                       name, type,
                       type == T_CNAME ? "CNAME" :
                       type == T_SRV ? "SRV" :
                       type == T_A ? "A" :
                       type == T_NAPTR ? "NAPTR" :
                       "unknown");
         break;
      }

      response = res_parse((char*) &answer);
      if (response == NULL)
      {
         // res_parse failed, return.
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "DNS query for name '%s', "
                       "type = %d (%s): response could not be parsed",
                       name, type,
                       type == T_CNAME ? "CNAME" :
                       type == T_SRV ? "SRV" :
                       type == T_A ? "A" :
                       type == T_NAPTR ? "NAPTR" :
                       "unknown");
         break;
      }
      // If requested for testing purposes, sort the query and print it.
      // Sort first, so we see how sorting came out.
      if (SipSrvLookup::getOption(SipSrvLookup::OptionCodeSortAnswers))
      {
         sort_answers(response);
      }
      if (SipSrvLookup::getOption(SipSrvLookup::OptionCodePrintAnswers))
      {
         res_print(response);
      }
      // Now that we have a fresh DNS query to analyze, go back and check it
      // for a CNAME for 'name' and then for records of the requested type.
   }

   // Final processing:  Copy the working name and response to the output
   // variables.
   out_name = name;
   out_response = response;
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSrvLookup::res_query_and_parse out_name = '%s', out_response = %p",
                 out_name, out_response);

}

/// Set the nameserver address to a specific nameserver.
void SipSrvLookup::set_nameserver_address(const char* ip,int port)
{
   mNameserverIP=ip;
   mNameserverPort=port;
}


union u_rdata* look_for(res_response* response, const char* name,
                        int type)
{
    unsigned i;

   for (i = 0; i < response->header.ancount; i++)
   {
      if (response->answer[i]->rclass == C_IN &&
          response->answer[i]->type == type &&
          strcasecmp(name, response->answer[i]->name) == 0)
      {
         return &response->answer[i]->rdata;
      }
   }
   for (i = 0; i < response->header.arcount; i++)
   {
      if (response->additional[i]->rclass == C_IN &&
          response->additional[i]->type == type &&
          strcasecmp(name, response->additional[i]->name) == 0)
      {
         return &response->additional[i]->rdata;
      }
   }
   return NULL;
}

// Functions to compare two server entries.
// sort server list so TCP forks will be tried last (UDP preferred)
int server_compare_prefer_udp(const void* a, const void* b)
{
    int result;
    result = server_compare(a, b,
                            OsSocket::TCP);  // TCP is least preferred transport
    return result;
}

// sort server list so UDP forks will be tried last (TCP preferred)
int server_compare_prefer_tcp(const void* a, const void* b)
{
    int result;
    result = server_compare(a, b,
                            OsSocket::UDP);  // UDP is least preferred transport
    return result;
}

int server_compare(const void* a, const void* b,
                   OsSocket::IpProtocolSocketType leastPreferredProto)
{
    int result = 0;
    const server_t* s1 = (const server_t*) a;
    const server_t* s2 = (const server_t*) b;

    /* First compare priorities.  Lower priority values are preferred, and
     * should go at the beginning of the list, and so should be returned
     * as less-than.
     */
    if (s1->priority > s2->priority)
    {
        result = 1;
    }
    else if (s1->priority < s2->priority)
    {
        result = -1;
    }
    // Next compare the scores derived from the weights.
    // With the new scheme for computing scores, lower score values should
    // sort to the beginning of the list, that is, should compare less than
    // higher scores.
    // See sipXtackLib/doc/developer/scores/README for details.
    else if (s1->score < s2->score)
    {
        result = -1;
    }
    else if (s1->score > s2->score)
    {
        result = 1;
    }
    // Compare the transport type, so everything is favored over leastPreferredProto.
    // That means that leastPreferredProto must be larger than others.
    // Smaller messages will ask to prefer UDP (TCP is lowest)
    // Large messages will ask to prefer TCP (UDP is lowest)
    // See SipTransaction::getPreferredProtocol.
    else if (s1->type == leastPreferredProto
          && s2->type != leastPreferredProto)
    {
        result = 1;
    }
    else if (s1->type != leastPreferredProto
          && s2->type == leastPreferredProto)
    {
        result = -1;
    }

    return result;
}

int server_compare_presort(const void* a, const void* b)
{
    int result = 0;
    const server_t* s1 = (const server_t*) a;
    const server_t* s2 = (const server_t*) b;

    // Compare s1 and s2 on all fields.
    result = strcmp(s1->host, s2->host);
    if (result == 0)
    {
       if (s1->type < s2->type)
       {
          result = -1;
       }
       else if (s1->type > s2->type)
       {
          result = 1;
       }
       else
       {
          result = memcmp(&s1->sin, &s2->sin, sizeof (s1->sin));
          if (result == 0)
          {
             if (s1->priority < s2->priority)
             {
                result = -1;
             }
             else if (s1->priority > s2->priority)
             {
                result = 1;
             }
             else if (s1->weight < s2->weight)
             {
                result = -1;
             }
             else if (s1->weight > s2->weight)
             {
                result = 1;
             }
          }
       }

    }
    return result;
}

/* //////////////////////////// server_t ///////////////////////////////// */

/// Initializer for server_t
server_t::server_t() :
   host(NULL)
{
}

// Copy constructor for server_t
server_t::server_t(const server_t& rserver_t) :
   host(rserver_t.host != NULL ? strdup(rserver_t.host) : NULL),
   type(rserver_t.type),
   sin(rserver_t.sin),
   priority(rserver_t.priority),
   weight(rserver_t.weight),
   score(rserver_t.score)
{
}

// Assignment operator for server_t
server_t& server_t::operator=(const server_t& rhs)
{
   // Handle the assignment-to-self case.
   if (this == &rhs)
   {
      return *this;
   }

   // Copy the host strign, if present.
   host = rhs.host != NULL ? strdup(rhs.host) : NULL;
   // Copy the other fields.
   type = rhs.type;
   sin = rhs.sin;
   priority = rhs.priority;
   weight = rhs.weight;
   score = rhs.score;

   return *this;
}

/// Destructor for server_t
server_t::~server_t()
{
   // All that needs to be done is free the host string, if any.
   if (host != NULL)
   {
      free(host);
   }
}

/// Inquire if this is a valid SRV record
UtlBoolean server_t::isValidServerT()
{
   // Entry is valid if host is not NULL.
   return host != NULL;
}

/// Accessor for host name
void server_t::getHostNameFromServerT(UtlString& hostName)
{
   hostName = (host != NULL) ? host : "";
}

/// Accessor for host IP address
void server_t::getIpAddressFromServerT(UtlString& hostName)
{
   OsSocket::inet_ntoa_pt(sin.sin_addr, hostName);
}

/// Accessor for port
int server_t::getPortFromServerT()
{
   return ntohs(sin.sin_port);
}

/// Accessor for weight
unsigned int server_t::getWeightFromServerT()
{
   return weight;
}

/// Accessor for score
float server_t::getScoreFromServerT()
{
   return score;
}

/// Accessor for priority
unsigned int server_t::getPriorityFromServerT()
{
   return priority;
}

/// Accessor for protocol
OsSocket::IpProtocolSocketType server_t::getProtocolFromServerT()
{
   return type;
}

/**
 * Post-process the results of res_parse by sorting the lists of "answer" and
 * "additional" RRs, so that responses are reproducible.  (named tends to
 * rotate multiple answer RRs to the same query.)
 */
static void sort_answers(res_response* response)
{
   qsort((void*) response->answer, response->header.ancount,
         sizeof (s_rr*), rr_compare);
   qsort((void*) response->additional, response->header.arcount,
         sizeof (s_rr*), rr_compare);
}

/**
 * Function to compare two RRs for qsort.
 *
 * I was hoping to sort records by TTL values, but Bind cleverly gives all
 * answers the same TTL (the minimum of the lot).  So we have to sort by
 * address (for A records) or port/target (for SRV records).
 */
static int rr_compare(const void* a, const void* b)
{
   int t;

   // a and b are pointers to entries in the array of s_rr*'s.
   // Get the pointers to the s_rr's:
   s_rr* a_rr = *(s_rr**) a;
   s_rr* b_rr = *(s_rr**) b;

   // Compare on type.
   t = a_rr->type - b_rr->type;
   if (t != 0)
   {
      return t;
   }

   // Case on type.
   switch (a_rr->type)
   {
   case T_SRV:
      // Compare on target.
      t = strcmp(a_rr->rdata.srv.target, b_rr->rdata.srv.target);
      if (t != 0)
      {
         return t;
      }
      // Compare on port.
      if (a_rr->rdata.srv.port < b_rr->rdata.srv.port)
      {
         return -1;
      }
      else if (a_rr->rdata.srv.port > b_rr->rdata.srv.port)
      {
         return 1;
      }
      // Give up.
      return 0;

   case T_A:
      // Compare on address.
      return memcmp((const void*) &a_rr->rdata.address,
                    (const void*) &b_rr->rdata.address,
                    sizeof (struct sockaddr));

   default:
      return 0;
   }
}


/* ////////////////////////// SrvThreadArgs /////////////////////////////// */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/// Constructor for SrvThreadArgs
SrvThreadArgs::SrvThreadArgs(const char * tmp_domain,
                             const char * tmp_service,
                             server_t*& tmp_list,
                             int tmp_list_length_allocated,
                             int tmp_list_length_used,
                             int tmp_port,
                             OsSocket::IpProtocolSocketType tmp_socketType) :
   OsMsg(SRV_LOOKUP_MSG, SRV_LOOKUP_MSG),
   list(tmp_list)
{
   port = tmp_port;
   socketType = tmp_socketType;

   domain = tmp_domain;
   service = tmp_service;
   list_length_allocated = new int(tmp_list_length_allocated);
   list_length_used = new int(tmp_list_length_used);

   mCopyOfObject = FALSE;
}

/// Copy Constructor for SrvThreadArgs
SrvThreadArgs::SrvThreadArgs(const SrvThreadArgs& rSrvThreadArgs) :
   OsMsg((OsMsg&) rSrvThreadArgs),
   list(rSrvThreadArgs.list)
{
   domain = rSrvThreadArgs.domain;
   service = rSrvThreadArgs.service;
   list_length_allocated = rSrvThreadArgs.list_length_allocated;
   list_length_used = rSrvThreadArgs.list_length_used;
   port = rSrvThreadArgs.port;
   socketType = rSrvThreadArgs.socketType;

   mCopyOfObject = TRUE;
}

/// Destructor for SrvThreadArgs
SrvThreadArgs::~SrvThreadArgs()
{
   // Do not delete these attributes as there maybe copies of the
   // object still using them. Only delete it when the original object is
   // being deleted
   if (!mCopyOfObject)
   {
      delete list_length_allocated;
      delete list_length_used;
   }
}

/// Create a copy of this msg object (which may be of a derived type)
OsMsg* SrvThreadArgs::createCopy(void) const
{
   return (new SrvThreadArgs(*this));
}


/* /////////////////////// SipSrvLookupThread ///////////////////////////// */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/*
 * Lock to protect the lookup routines, which cannot tolerate multi-threaded
 * use.
 */
OsMutex SipSrvLookupThread::slookupThreadMutex(OsMutex::Q_FIFO |
                                         OsMutex::DELETE_SAFE |
                                         OsMutex::INVERSION_SAFE);

/// Array holding pointers to the four individual lookup threads
SipSrvLookupThread * SipSrvLookupThread::mLookupThreads[SipSrvLookupThread::LAST_LookupType+1];

UtlBoolean SipSrvLookupThread::mHaveThreadsBeenInitialized = FALSE;

/// Destructor for SipSrvLookupThread
SipSrvLookupThread::~SipSrvLookupThread()
{

}

/// Implementation of OsServerTask's pure virtual method
UtlBoolean SipSrvLookupThread::handleMessage(OsMsg& rMsg)
{
   if (OsMsg::OS_SHUTDOWN == rMsg.getMsgType())
   {
      OsTask::requestShutdown();
   }
   else if (SRV_LOOKUP_MSG == rMsg.getMsgType())
   {
      SrvThreadArgs * mySrvArgs = dynamic_cast <SrvThreadArgs*> (&rMsg);

      switch (mLookupType)
      {
      case SRV_UDP:
         lookup_SRV(mySrvArgs->list, *(mySrvArgs->list_length_allocated),
               *(mySrvArgs->list_length_used), mySrvArgs->domain.data(),
               mySrvArgs->service.data(), "udp", OsSocket::UDP);
         break;

      case SRV_TCP:
         lookup_SRV(mySrvArgs->list, *(mySrvArgs->list_length_allocated),
               *(mySrvArgs->list_length_used), mySrvArgs->domain.data(),
               mySrvArgs->service.data(), "tcp", OsSocket::TCP);
         break;

      case SRV_TLS:
         lookup_SRV(mySrvArgs->list, *(mySrvArgs->list_length_allocated),
               *(mySrvArgs->list_length_used), mySrvArgs->domain.data(),
               mySrvArgs->service.data(), "tls", OsSocket::SSL_SOCKET);
         break;

      case A_RECORD:
         lookup_A(mySrvArgs->list, *(mySrvArgs->list_length_allocated),
               *(mySrvArgs->list_length_used), mySrvArgs->domain.data(),
               // If sips service, make sure transport is set correctly.
               (mySrvArgs->socketType == OsSocket::UNKNOWN && strcmp(mySrvArgs->service.data(),
                     "sips") == 0) ? OsSocket::SSL_SOCKET : mySrvArgs->socketType, NULL, // must do a query.
               (portIsValid(mySrvArgs->port) ? mySrvArgs->port : // use port if specified
                     ((strcmp(mySrvArgs->service.data(), "sips") == 0) || // else use 5061 for sips;
                           (mySrvArgs->socketType == OsSocket::SSL_SOCKET)) ? 5061 : 5060), // else use 5060.
               0, 0); // Set priority and weight to 0.

         break;

      }
   }

   mQueryCompleted->signal(0);

   return TRUE;
}

/// Get the SRV lookup threads, initializing them if needed
SipSrvLookupThread ** SipSrvLookupThread::getLookupThreads()
{
   OsLock lock(SipSrvLookupThread::slookupThreadMutex);

   if (!mHaveThreadsBeenInitialized)
   {
      for(int x = FIRST_LookupType; x <= LAST_LookupType; x++)
      {
         mLookupThreads[(LookupTypes) x] = new SipSrvLookupThread((LookupTypes) x);
         mLookupThreads[(LookupTypes) x]->start();
      }
      mHaveThreadsBeenInitialized = TRUE;
   }

   return mLookupThreads;
}

/// Block until the thread has finished a query
void SipSrvLookupThread::isDone()
{
   mQueryCompleted->wait();
   mQueryCompleted->reset();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/*
 * Private constructor for SipSrvLookupThread
 * Use getLookupThreads() for initializing and accessing the class members. The class
 * is restricted to 4 members by the getLookupThreads() function, and thier pointers
 * are stored in "mLookupThreads"
 */

SipSrvLookupThread::SipSrvLookupThread(LookupTypes lookupType) :
   OsServerTask("SipSrvLookupThread%d")
{
   mLookupType = lookupType;
   mQueryCompleted = new OsEvent();
   mQueryCompleted->reset();
}
