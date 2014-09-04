//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipProtocolServerBase_h_
#define _SipProtocolServerBase_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipClient.h>
#include <os/OsServerSocket.h>
#include <os/OsTask.h>
#include <os/OsServerTask.h>
#include <os/OsLockingList.h>
#include <os/OsRWMutex.h>
#include <utl/UtlHashMap.h>
#include <Poco/Semaphore.h>
#include <os/OsThreadPool.h>
#include <list>
#include <boost/thread.hpp>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipProtocolServerBase : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

  typedef boost::mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;
  typedef std::list<SipClient::Ptr> SipClientList;
  
   enum EventSubTypes
   {
      SIP_SERVER_GC = 1
   };

/* ============================ CREATORS ================================== */

   SipProtocolServerBase(SipUserAgent* userAgent,
                         const char* protocolString,
                         const char* taskName);
   //:Default constructor

   virtual
      ~SipProtocolServerBase();
   //:Destructor

/* ============================ MANIPULATORS ============================== */

   /// Handles an incoming message.
   virtual UtlBoolean handleMessage(OsMsg& rMsg);

   // Manipulates the server list, and so should be called only by the Sip*Server
   // constructors/destructors and the threads that call them.
   virtual UtlBoolean startListener();

   // Manipulates the server list, and so should be called only by the Sip*Server
   // constructors/destructors and the threads that call them.
   virtual void shutdownListener() = 0;

   UtlBoolean send(SipMessage* message,
                   const char* hostAddress,
                   int hostPort = SIP_PORT);

   virtual int run(void* pArg) = 0;

   void removeOldClients(long oldTime);

/* ============================ ACCESSORS ================================= */

   virtual int isOk();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   int getClientCount();

   virtual void printStatus();

   /** Find the SipClient for this address, and create one if it does not
    *  exist.
    */
   // Returns NULL if it is unable to create a client.
   
   SipClient::Ptr getClientForDestination(const UtlString& hostAddress, int hostPort, const UtlString& localIp);

   void startClients();

   void shutdownClients();

   UtlBoolean clientExists(SipClient* client);

   void addClient(const SipClient::Ptr& pClient);

   /// Create a socket suitable for use by a SipClient.
   //  Must be non-blocking.
   virtual OsSocket* buildClientSocket(int hostPort,
                                       const char* hostAddress,
                                       const char* localIp,
                                       bool& existingSocketReused) = 0;

   UtlString mProtocolString;
   UtlString mDefaultIp;
   int mDefaultPort;

   /// The owning SipUserAgent, if any.
   SipUserAgent* mSipUserAgent;

   // Description of the server (listening) SipClient's.
   // These variables are changed only by the Sip*Server
   // constructors/destructors and the threads that call them.
   // Map from the local IP addresses (as UtlString's) to
   // OsSocket's listening on those addresses.
   // The OsSocket's are owned by the SipServerBroker's in mServers,
   // and the key strings are the same as those in (and owned by
   // mServerPortMap).
   UtlHashMap mServerSocketMap;
   // Map from the local IP addresses (as UtlString's) to UtlInt's of the
   // listening port numbers.
   UtlHashMap mServerPortMap;
   // Map from the local IP addresses (as UtlString's) to the listening
   // SipClient's.
   UtlHashMap mServers;

 
   SipClient::Ptr findExistingClientForDestination(const UtlString& hostAddress, int hostPort, const UtlString& localIp);

   void deleteClient(SipClient* client);
   
   void handleSend(SipMessage* pMsg);


   bool mIsSecureTransport;
   
   SipClientList _clientList;
   mutex _clientMutex;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipProtocolServerBase(const SipProtocolServerBase& rSipProtocolServerBase);
   //: disable Copy constructor

   SipProtocolServerBase& operator=(const SipProtocolServerBase& rhs);
   //:disable Assignment operator
public:
  OsThreadPool<SipMessage*> _threadPool;
  Poco::Semaphore _threadPoolSem;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipProtocolServerBase_h_
