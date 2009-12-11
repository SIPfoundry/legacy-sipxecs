//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipSrvLookup_h_
#define _SipSrvLookup_h_

// SYSTEM INCLUDES
#if defined(_WIN32)
//#   include <resparse/wnt/netinet/in.h>
#elif defined(_VXWORKS)
#  include <netinet/in.h>
#elif defined(__pingtel_on_posix__)
#   include <netinet/in.h>
#else
#  error Unsupported target platform.
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMutex.h"
#include "os/OsSocket.h"
#include "os/OsServerTask.h"
#include "os/OsMsg.h"
#include "os/OsEvent.h"

// DEFINES
#define SRV_LOOKUP_MSG OsMsg::USER_START

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

// TYPEDEFS

// FORWARD DECLARATIONS
class server_t;
class SipSrvLookupThread;
class SrvThreadArgs;
typedef struct s_res_response
    res_response;

/**
 * A class (with no members) whose 'servers' method implements the RFC
 * 3263 process for determining a list of server entries for a SIP
 * domain name.
 */
class SipSrvLookup
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Get the list of server entries for SIP domain name 'domain'.
   static server_t* servers(const char *domain,
                            ///< SIP domain name or host name
                            const char *service,
                            ///< "sip" or "sips"
                            OsSocket::IpProtocolSocketType socketType,
                            ///< types of transport
                            int port,
                            ///< port number from URI, or PORT_NONE
                            OsSocket::IpProtocolSocketType preferredTransport = OsSocket::UDP
                            ///< preferred transport protocol
      );
   /**<
    * Returns the ordered list of server entries for SIP domain name 'domain'.
    * Implements the processes of RFC 2543.
    * The search process is modified by the parameters:
    *
    * The 'service' is the URI scheme, "sip" or "sips".
    *
    * The 'port' argument is the port number supplied in the URI, or a
    * negative number if none was supplied.
    * If none was supplied, SRV records are consulted.  If no SRV records
    * are found, port defaults to 5060 for "sip" service and 5061 for
    * "sips" service.
    *
    * The 'socketType' restricts the search to that sort of transport,
    * and may have the values (within IpProtocolSocketType):
    *    TCP
    *    UDP
    *    SSL_SOCKET
    *    UNKNOWN (all of the above are acceptable)
    * It is used if the URI or other context information specifies a transport.
    * If UNKNOWN is specified, servers() attempts to return addresses for
    * all transports that it knows about that are compatible with the service.
    * (If socketType is incompatible with service, no addresses will
    * be returned.)
    *
    * The 'preferredTransport' specifies how to sort servers with respect to
    * transport protocols and may have the values (within IpProtocolSocketType):
    *    TCP
    *    UDP    (default value)
    * RFC-3261 specifies TCP should be preferred for larger messages
    *
    * The standard ordering of SRV records can be biased by passing a name
    * to setOwnHostname.  If SRV records are used, and this name matches one
    * of a set of alternative servers that would otherwise be randomly selected
    * based on weights, then this name will always be the first among those
    * alternatives.  Note that this does not affect selection based on priorities
    * or transports.
    *
    * @returns Allocates an array of server_t objects and returns
    * the pointer to it.  Caller is responsible for delete[]'ing the array.
    * The servers are listed in the array in preference order, with a final
    * entry with a host value of NULL.
    */

   /// Option codes for server lookup.
   enum OptionCode {
      OptionCodeNone = 0,       ///< Special value
      OptionCodeFirst,          ///< Start of range
      OptionCodeIgnoreSRV,      ///< If 1, do not search for SRV records.
      OptionCodeIgnoreNAPTR,    ///< If 1, do not search for NAPTR records.
      OptionCodeSortAnswers,    /**< If 1, sort DNS answers before using them.
                                 *   (For testing only.) */
      OptionCodePrintAnswers,   /**< If 1, print DNS answers.
                                 *   (For testing only.) */
      OptionCodeCNAMELimit,     ///< Max. number of CNAMEs to follow.
      OptionCodeNoDefaultTCP,   /**< If 1, do not add TCP contacts by default,
                                 *   for better RFC 3263 section 4.1 conformance. */
      OptionCodeSortServers,    /**< If 1, sort the server list before
                                 *   calculating scores.  This makes the scores
                                 *   predictable.  (For testing only.) */
      OptionCodeLast,           ///< End of range
   };
   /**<
    * All options have a code name in this enumeration.  All codes are in the
    * range OptionCodeFirst:OptionCodeLast.  OptionCodeNone is not in that
    * range and may be used as a special value.
    *
    * Option values are signed integers.
    *
    * Getting and setting all option codes in that range is guaranteed to
    * restore the server lookup algorithm to the preceeding codition.
    *
    * The way enum values are assigned is somewhat sloppy (it allows some
    * code values that are not used to be in the valid range), but it makes
    * editing the enum correctly quite easy.  Do not reorder the list without
    * checking the initializer in SipSrvLookup.cpp; some options have non-zero
    * initial values.
    */

   /// Get an option value.
   static inline int getOption(OptionCode option)
      {
         return options[option];
      }

   /// Set an option value.
   static void setOption(OptionCode option, int value);
   /**
    * setOption can be called at any time from any thread without
    * causing harm to any requests that may be being processed.
    * It may, however, block until all current requests are processed
    * before returning to the caller, and current requests may be processed
    * partly under the old configuration and partly under the new
    * configuration.
    */

   /// Sets our own hostname - if selection is by weight, our own name sorts above other alternatives
   static void setOwnHostname(const char* hostname);

   /// Is the passed host name the same as that passed to setOwnHostname?
   static bool isOwnHostname(const char* hostname);

   /// Sets the timing parameters for DNS SRV queries.
   static void setDnsSrvTimeouts(int initialTimeoutInSecs,
                                 /**< Timeout in seconds for first query,
                                  *   or 0 for no change. */
                                 int retries
                                 /**< Number of retries to attempt,
                                  *   or 0 for no change. */
      );
   ///< Defaults are: timeout = 3, retries = 2.

   /// Perform a DNS query and parse the results.  Follows CNAME records.
   static void res_query_and_parse(const char* in_name,
                                   ///< domain name to look up
                                   int type,
                                   ///< RR type to look up
                                   res_response* in_response,
                                   /**< response structure to
                                    *   look in before calling
                                    *   res_query, or NULL */
                                   const char*& out_name,
                                   ///< canonical name for in_name
                                   res_response*& out_response
                                   ///< response structure containing RRs
      );
   /**<
    * Performs a DNS query for a particular type of RR on a given name,
    * doing all the work to follow CNAMEs.  The 'in_name' and 'type'
    * arguments specify the RRs to look for.  If 'in_response' is not NULL,
    * it is the results of some previous search for the same name, for
    * a different type of RR, which might contain RRs for this search.
    *
    * @return out_response is a pointer to a response structure, or NULL.
    * If non-NULL, the RRs of the required type (if any) are in out_response
    * (in either the answer section or the additional section), under the name
    * out_name.
    *
    * The caller is responsible for freeing out_name if it is non-NULL
    * and != in_name.  The caller is responsible for freeing out_response if it
    * is non-NULL and != in_response.
    */

    /// Set the nameserver address to a specific nameserver.
    static void set_nameserver_address(const char* ip,
                                       ///< IP address of the nameserver
                                       int port
                                       ///< The port address of the nameserver
    );

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// Mutex to keep the routines thread-safe.
   static OsMutex sMutex;

   /// The array of option values.
   static int options[OptionCodeLast+1];

   /// Our own hostname
   static UtlString mOwnHostname;
   
   /// Sets the timeout parameter for DNS SRV queries. Default is 3
   static int mTimeout;

   /// Sets the number of retries for DNS SRV queries. Default is 2
   static int mRetries;

   /// Sets the IP address of the nameserver. If NULL we use the default Nameservers
   static UtlString mNameserverIP;

   /// Sets the port number  of the nameserver. If NULL we use the default port number 53
   static int mNameserverPort;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};


/**
 * Structure to describe a server found for a SIP domain.
 */
class server_t {
  public:

   //! Sets the mDnsSrvResolveEnabled flag
   static void setDnsSrvResolveEnabled(UtlBoolean& enabled);

   char *host;                  ///< Host name. (Owned by this object.)
   OsSocket::IpProtocolSocketType type;
                                ///< OsSocket:{TCP,UDP,SSL_SOCKET}
   struct sockaddr_in sin;      ///< IP address and port
   unsigned int priority;       ///< SRV priority value
   unsigned int weight;         ///< SRV weight
   float score;                 ///< Calculated sorting score

   /// Initializer for server_t
   server_t();
   static UtlBoolean mDnsSrvResolveEnabled;

   /// Copy constructor for server_t
   server_t(const server_t& rserver_t);

   /// Copy assignment constructor for server_t
   server_t& operator=(const server_t& rhs);

   /// Destructor for server_t
   ~server_t();

   /// Inquire if this is a valid SRV record
   UtlBoolean isValidServerT();
   ///< Tests whether the host name element is non-NULL.

   /// Accessor for host name
   void getHostNameFromServerT(UtlString& hostName);

   /// Accessor for host IP address
   void getIpAddressFromServerT(UtlString& hostName);

   /// Accessor for port
   int getPortFromServerT();

   /// Accessor for weight
   unsigned int getWeightFromServerT();

   /// Accessor for score
   float getScoreFromServerT();

   /// Accessor for priority
   unsigned int getPriorityFromServerT();

   /// Accessor for protocol
   OsSocket::IpProtocolSocketType getProtocolFromServerT();
};

/**
 * A class derived from OsServerTask class, whose members are responsible for carrying out
 * DNS queries. The class is restricted to a total of 4 members, with each member responsible
 * for the DNS query type specified by "mLookupType".
 */

class SipSrvLookupThread: public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Enum indicating the 4 different lookup types we carry out
   enum LookupTypes
   {
      FIRST_LookupType = 0,
      SRV_UDP = FIRST_LookupType,
      SRV_TCP,
      SRV_TLS,
      A_RECORD,
      LAST_LookupType = A_RECORD
   };

   /// Destructor for SipSrvLookupThread
   virtual ~SipSrvLookupThread(void);

   /// Get the SRV lookup threads, initializing them if needed
   static SipSrvLookupThread** getLookupThreads();

   /// Implementation of OsServerTask's pure virtual method
   UtlBoolean handleMessage(OsMsg& rMsg);

   /// Mutex to keep the Lookups thread-safe.
   static OsMutex slookupThreadMutex;

   /// Block until the thread has finished a query
   void isDone();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Private constructor for SipSrvLookupThread
   SipSrvLookupThread(LookupTypes lookupType);
   /**<
    * Use getLookupThreads() for initializing and accessing the class members. The class
    * is restricted to 4 members by the getLookupThreads() function, and their pointers
    * are stored in "mLookupThreads"
    */

   /// Class attribute indicating what type of query this class member is responsible for
   LookupTypes mLookupType;

   /// Array holding pointers to the four individual lookup threads
   static SipSrvLookupThread* mLookupThreads[LAST_LookupType+1];

   /// Attribute indicating whether the lookup threads have been initialized or not
   static UtlBoolean mHaveThreadsBeenInitialized;

   /// Events used to signal the completion of a query
   OsEvent* mQueryCompleted;

};

/**
 * A class derived from OsMsg class, whose members store the arguments necessary  to
 * perform the DNS queries, store the result, and signal an event to the main thread
 * to indicate the completion of a query.
 */

class SrvThreadArgs: public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Constructor for SrvThreadArgs
   SrvThreadArgs(const char * tmp_domain,
                 ///< SIP domain name or host name
                 const char * tmp_service,
                 ///< "sip" or "sips"
                 server_t*& tmp_list,
                 ///< Array of objects which describe a server found for a SIP domain
                 int tmp_list_length_allocated,
                 ///< Length of the array of server_t objects
                 int tmp_list_length_used,
                 ///< Number of objects that have been used
                 int tmp_port,
                 ///< port number from URI, or PORT_NONE
                 OsSocket::IpProtocolSocketType tmp_socketType
                 ///< preferred transport protocol
    );


   /// Copy constructor for SrvThreadArgs
   SrvThreadArgs(const SrvThreadArgs& rSrvThreadArgs);

   /// Destructor for SrvThreadArgs
   virtual ~SrvThreadArgs();

   /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const;

   /// Argument needed by the lookup threads
   server_t*& list;
   /**<
    * Array of objects which describe a server found for a SIP domain
    * This attribute is shared between all the lookup threads. Access to it
    * is controlled via the "sLookupThreadMutex" mutex in the SipSrvLookupThread class
    */

   /// Argument needed by the lookup threads
   int* list_length_allocated;
   /**<
    * Length of the array of server_t objects
    * This attribute is shared between all the lookup threads. Access to it
    * is controlled via the "sLookupThreadMutex" mutex in the SipSrvLookupThread class
    */

   /// Argument needed by the lookup threads
   int* list_length_used;
   /**<
    * Number of objects in the server_t object array that have been used
    * This attribute is shared between all the lookup threads. Access to it
    * is controlled via the "sLookupThreadMutex" mutex in the SipSrvLookupThread class
    */

   /// Argument passed to the lookup threads by copy. No access control on this.
   UtlString domain;

   /// Argument passed to the lookup threads by copy. No access control on this.
   UtlString service;

   /// Argument passed to the lookup threads by copy. No access control on this.
   int port;

   /// Argument passed to the lookup threads by copy. No access control on this.
   OsSocket::IpProtocolSocketType socketType;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /// Private attribute to keep track of object copies.
   UtlBoolean mCopyOfObject;

};

#endif  // _SipSrvLookup_h_
