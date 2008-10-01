// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTSERVER_H
#define SIPREDIRECTSERVER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "digitmaps/UrlMapping.h"
#include "os/OsConfigDb.h"
#include "utl/UtlHashMap.h"
#include "registry/RedirectPlugin.h"
#include "registry/RedirectSuspend.h"
#include "net/SipUserAgent.h"
#include "utl/UtlHashMapIterator.h"
#include "os/OsMutex.h"
#include "utl/PluginHooks.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;
class SipRedirectServerPrivateStorageIterator;

class SipRedirectServer : public OsServerTask  
{
   friend class SipRedirectServerPrivateStorageIterator;

  public:

   SipRedirectServer(OsConfigDb*   pOsConfigDb,  ///< Configuration parameters
                     SipUserAgent* pSipUserAgent ///< User Agent to use when sending responses
                     );

   virtual ~SipRedirectServer();

   static SipRedirectServer* getInstance();
   static SipRedirectServer* spInstance;

   /// Initiate cleanup and shutdown.
   virtual void requestShutdown(void);
   
   /// Read in the configuration for the redirect server.
   UtlBoolean initialize( OsConfigDb& configDb    ///< Configuration parameters
                         );

   /**
    * Used by redirector asynchronous processing to request that
    * processing a request be resumed.
    *
    * May be called from any context.  Does not block.
    *
    * requestSeqNo - request sequence number
    *
    * redirectorNo - the number of this redirector
    */
   void resumeRequest(
      RedirectPlugin::RequestSeqNo requestSeqNo,
      int redirectorNo);

   /**
    * Look up the private storage for a particular request.
    *
    * Caller must hold mMutex, and keep it until it discards the
    * return value.
    *
    * requestSeqNo - request sequence number.  The request must have
    * been previously suspended.
    *
    * redirectorNo - the number of this redirector
    *
    * @return Pointer to the private storage area for this request and
    * redirector, or NULL.
    */
   SipRedirectorPrivateStorage* getPrivateStorage(
      RedirectPlugin::RequestSeqNo requestSeqNo,
      int redirectorNo);

   /**
    * Lock that is global for this SipRedirectServer to protect
    * mSuspendList and the private storage dependend from it.
    */
   OsMutex mRedirectorMutex;

  protected:

   UtlBoolean mIsStarted;
   SipUserAgent* mpSipUserAgent;
   SipRegistrar* mpRegistrar;
   
   UrlMapping mFallback;
   OsStatus mFallbackRulesLoaded;
   // A port number, which if found on an AOR to register,
   // will be removed, or PORT_NONE
   int mProxyNormalPort;
   // The Route header address and parameter value(lr) to use to  
   // "forward" redirected ACKs "back" to the proxy.  
   UtlString mAckRouteToProxy;

   // functions
   UtlBoolean handleMessage(OsMsg& eventMessage);

   // The sequence number for the next request.
   RedirectPlugin::RequestSeqNo mNextSeqNo;

   // The list of all requests that have been suspended.
   UtlHashMap mSuspendList;

   // Service functions.
   void processRedirect(const SipMessage* message,
                        UtlString& method,
                        RedirectPlugin::RequestSeqNo seqNo,
                        RedirectSuspend* suspendObject);

   void cancelRedirect(UtlInt& containableSeqNo,
                       RedirectSuspend* suspendObject);

   // Members to manage the set of redirector plugins.

   // The PluginHooks object for managing the list of redirector plugins.
   PluginHooks mRedirectPlugins;
   // The number of redirectors.
   int mRedirectorCount;

   /** Pointer to an array of UtlBoolean's that record whether redirectors
    *  are active, based on the return values of ::initialize().
    */
   UtlBoolean* mpActive;
   
   // SIP domain that registrar/redirect server is authoritative for
   UtlString mDefaultDomain;
   
  private:
     /**
      *  Fills in the supplied response with the common fields taken from the 
      *  request and and the data contained in the supplied ErrorDescriptor 
      */
     void buildResponseFromRequestAndErrorDescriptor( SipMessage& response, 
                                                      const SipMessage& request, 
                                                      const ErrorDescriptor& errorDescriptor );
    
     friend class SipRedirectServerTest;
};

/**
 * Iterator that returns the request numbers and private storage pointers
 * for all suspended requests.
 *
 * It has a limited set of operations to avoid unpleasant interactions
 * with the rest of the suspend/resume mechanism.
 *
 * Caller must hold mMutex, and keep it until it discards the
 * returned iterator and any pointers obtained from it.
 *
 * redirectorNo - the number of this redirector
 *
 * <p>
 * Example Code:
 * <pre>
 *    // Seize the global lock.
 *    OsLock lock(SipRedirectServer::getInstance()->mMutex);
 *
 *    // Create an iterator that walks through the suspended requests
 *    // and returns the private storage pointers for a chosen redirector.
 *    SipRedirectorPrivateStorageIterator itor(redirectorNo);
 *
 *    // Fetch a pointer to each element of myContentSource into pStorage.
 *    SipRedirectorPrivateStorage* pStorage;
 *    while (pStorage = itor())
 *    {
 *       // Do something to *pStorage.
 *       ...
 *       // Get the request sequence number for this request.
 *       requestSeqNo = itor.requestSeqNo();
 *    }
 * </pre>
 */
class SipRedirectServerPrivateStorageIterator : protected UtlHashMapIterator
{
  public:

/* ============================ CREATORS ================================== */

   /**
    * Constructor accepts the redirector number for which we are to
    * find the storage.  It finds the SipRedirectServer via
    * SipRedirectServer::getInstance().
    */
   SipRedirectServerPrivateStorageIterator(
      int redirectorNo);

/* ============================ MANIPULATORS ============================== */

   /**
    * Return the private storage pointer for the next request.
    *
    * @return The private storage pointer for the redirector specified in the
    * constructor.  Iterates through the list until it finds the next
    * suspended redirection request which has a non-NULL pointer for this
    * redirector.
    */
   virtual SipRedirectorPrivateStorage* operator()();

/* ============================ ACCESSORS ================================= */

   /**
    * Gets the request sequence number of the current request.
    *
    * @return The request sequence number of the suspended redirection
    * request that the iterator has just returned.
    */
   RedirectPlugin::RequestSeqNo requestSeqNo() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   /**
    * The redirector number.
    */
   int mRedirectorNo;

};

#endif // SIPREDIRECTSERVER_H
