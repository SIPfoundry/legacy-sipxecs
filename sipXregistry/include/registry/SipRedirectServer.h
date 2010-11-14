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

/**
 * <b>Redirection Overview</b>
 *
 *  The sipXregistry redirect server implemented by the SipRedirectServer class
 *  maps the Request-URI of an incoming request to a set of alternative locations
 *  at which the target of that URI can be found.   This mapping task is accomplished
 *  with the help of Redirect plug-ins that are called upon in succession by the
 *  SipRedirectServer to look up the request and contribute to the circulated ContactList.
 *  When this process completes, if non-empty, the resulting ContactList contains all
 *  the contacts that the request should be redirected to and will be furnished to the
 *  requester in a "302 Moved Temporarily" response.  If empty, a "404 Not Found" is sent
 *  in response to the request.
 *
 *  Each redirect plug-in is a concrete implementation of the RedirectPlugin
 *  interface and must conform to its specification. @See RedirectPlugin for additional
 *  information.
 *
 * <b>Order and precedence of redirect plug-ins</b>
 *
 *  In order to establish their relative order and precendence, each redirect plug-in has two
 *  attributes:
 *   -# a three-digit ordinal number which establishes the order;
 *   -# an authority level which establishes the precedence.
 *
 * <b>Effects of the ordinal number attribute</b>
 *
 *  The ordinal number attribute is used by the SipRedirectServer to determine the order
 *  in which it is going to call the redirect plug-ins to redirect a given incoming request.
 *  That order is established based on the following rules:
 *
 *  Let 'A' and 'B' be redirect plug-ins 'A' and 'B'.
 *  Let ord(A) and ord(B) be the ordinal number attributes of redirect plug-ins 'A' and 'B'.
 *
 *  - A will be called before B if ord(A) < ord(B)
 *  - A will be called after B if ord(A) > ord(B)
 *  - The calling order of A and B is non-deterministic if ord(A) == ord(B)
 *
 * <b>Effects of the authority level attribute</b>
 *
 *  The authority level attribute is used by the SipRedirectServer to establish the precedence
 *  of the redirect plug-ins according to these rules.  If a redirect plug-in has a sufficiently
 *  high authority level, it will offered the opportunity to provide Contacts for the request via a call
 *  to its RedirectPlugin::lookUp() method otherwise it will ony be allowed to observe the request
 *  via a call to its RedirectPlugin::observe().
 *
 *  Let A be the last redirect plug-in that modified the ContactList.
 *  Let B be a redirect plug-in that comes after A (immediately or not) in the redirect
 *  plug-in chain ( implies that ord(A) < ord(B) ).
 *  Let AL(A) and AL(B) be the authority level attributes of redirect plug-ins 'A 'and 'B'
 *
 *   - B will be allowed to look up the request and modify the ContactList if
 *     it chooses to iff AL(B) >= AL(A)
 *   - B will be allowed to observe the request and ContactList without being able
 *     to modify it iff AL(B) < AL(A)
 *
 *  To state it in words, when a plug-in is called upon by SipRedirectServer to look up a
 *  request, it means that its authority level is higher than or equal to the authority level
 *  of the last plug-in to have modified the ContactList.  This plug-in is therefore
 *  granted the opportunity to modify the ContactList the way it sees fit to satisfy
 *  its redirection policy wihtout any regards to the plug-ins that contributed to
 *  the list before it.  If the plug-in modifies the ContactList in any way either
 *  by adding new contact(s), modifying or removing already contributed contacts or
 *  simply by 'touch'ing the ContactList, that list ,in a sense, inherits the autority
 *  level of that plug-in such that it can only be further modified by a subsequent
 *  redirect plug-in of greater or equal authority level. Plug-ins that have an
 *  authority level inferior to the last plug-in that modfied the ContactList will only
 *  be allowed to observe the request but not to contribute to the ContactList.
 *
 * <b> Possible uses of order and authority level attributes to arbitrate interactions
 *     between plug-ins</b>
 *
 *  Having each plug-in associated with ordinal and authority levels creates a
 *  two-dimensional framework within which some redirect plug-in interactions can be managed.
 *  This section will provide three such examples:
 *
 *  <b>Case #1</b> - Mutually-exclusive plug-ins
 *
 *  Let 'A' and 'B' be redirect plug-ins 'A' and 'B'.
 *  Let B be governed by a redirection policy such that it should not provide contacts
 *  if A already did.
 *
 *  This interaction between A and B can be managed through proper configuration
 *  of the ordinal and authority level attributes of A and B as follows:
 *
 *   - Set Ord(A) < Ord(B)
 *   - Set AL(A)  > AL(B)
 *
 *  <b>Case #2</b> - "editor" plug-in
 *
 *  Let 'A' be governed by a redirection policy such that it must inspect and optionally
 *  edit all the contacts contributed by the plug-ins before SipRedirectServer sends out
 *  its "302 Moved Temporarily"
 *
 *  This interaction between A and the rest of the plug-ins can be managed through proper
 *  configuration of the ordinal and authority level attributes of the plug-ins as follows:
 *
 *   - Set Ord(A) > all other plug-ins
 *   - Set AL(A)  > all other plug-ins
 *
 *  <b>Case #3</b> - Specialized Authoritative Plug-in
 *
 *  Let 'A' be governed by a redirection policy such that it is responsible for handling
 *  a specialized subset of requests and when it does, the contacts it supplies are final.
 *  A theoretical example of such a plug-in could be one that is responsible for redirecting
 *  emergency calls to the proper location.
 *
 *  This interaction between A and the rest of the plug-ins can be managed through proper
 *  configuration of the  authority level attribute of the plug-ins as follows:
 *
 *   - Set Ord(A) == Don't care[1]
 *   - Set AL(A)  > all other plug-ins
 *
 *  [1] Any value of Ord(A) will achieve the goal but in most cases, performance gains can
 *      be realized by using Ord(A) values that would put it at (or towards) the beginning
 *      of the redirect plug-in chain.
 *
 * <b>Error Handling</b>
 *
 *  When a redirect plug-in is asked to look-up a request, that plug-in, upon inspection
 *  of the request, may determine that the request is interesting but contains errors
 *  that prevent it from completing the look-up operation.  When such conditions
 *  are encountered, the plug-in will return the ERROR status code.  When the SipRedirectServer
 *  sees a plug-in returning ERROR, it immediately aborts the redirect plug-in chain and
 *  builds a final failure response based on the returned ErrorDescriptor.
 *
 * <b>Asynchronous processing of requests by redirect plug-ins</b>
 *
 *  When a redirect plug-in looks up a request, it returns a status code.  The
 *  values are SUCCESS, ERROR (which means that redirection should
 *  terminate immediately), and SEARCH_PENDING.  If the redirect
 *  plug-in returns SEARCH_PENDING, it is indicating that its processing
 *  cannot be completed quickly.
 *
 *  Before returning SEARCH_PENDING, the redirect plug-in is responsible for arranging
 *  for asynchronous processing that will bring the redirect plug-in into a
 *  state where it can complete its processing quickly.  Asynchronous
 *  processing then calls RedirectPlugin::resumeRedirection() to indicate to the
 *  SipRedirectServer that the request should be reprocessed.
 *
 *  The SipRedirectServer will the re-execute the redirect plug-in chain for the
 *  request.  The redirect plug-ins are expected to succeed, after which the
 *  SipRedirectServer sends a response to the request.  (A redirect plug-in is
 *  allowed to return SEARCH_PENDING several times for a single request, but
 *  this should be avoided for efficiency.)
 *
 *  A redirect plug-in has a cancel() method by which the SipRedirectServer may
 *  inform it that a request that it is working on has been canceled.  Such
 *  cancellation may be due to a CANCEL from the requester, a transaction
 *  time-out (because some redirect plug-in remained suspended too long),
 *  reinitialization of the SipRedirectServer, or an error response from
 *  another redirector.
 *
 *  If a redirect plug-in asks for suspension, processing continues with the
 *  other redirect plug-ins in the sequence, but at the end of processing, the
 *  ContactList that is being generated is discarded, and the entire sequence
 *  is re-executed once all the redirect plug-ins have indicated that they can
 *  succeed, generating a new, correct ContactList.  This is somewhat
 *  inefficient, but there is no simple scheme to make it more efficient
 *  that does not place tight constraints on the redirect plug-ins that we may
 *  want to loosen in the near future.  The current target is that a
 *  suspended redirection should take no more than twice the work of a
 *  non-suspended redirection, and that a very small fraction of
 *  redirection requests will be suspended.
 *
 *  Each request that is suspended has a sequence number that is unique
 *  over long periods of time.  Currently, the sequence number is a 32 bit
 *  unsigned integer that increments for every request.  It is given to
 *  each redirector and is used to identify the request.
 *
 *  redirect plug-ins may maintain their own data storage in one of two ways.
 *
 *  The first method is simple to code but is single-threaded.  The
 *  SipRedirectServer maintains for every suspended request and every
 *  redirect plug-in a pointer to private storage for that redirect plug-in.  The
 *  redirect plug-in may allocate an object and save the pointer to it.  The
 *  SipRedirectServer guarantees to delete the object (thus executing its
 *  destructor) after processing of the request is done.  Using service
 *  methods, the redirect plug-in can find the object for any particular
 *  sequence number, or examine the objests for all suspended requests.
 *  However, any access to these objects requires holding a global mutex.
 *
 *  The second method is more complex to code but can be parallelized.
 *  That method is for the redirector to maintain its own data structures,
 *  using the request serial numbers to coordinate its data items with the
 *  SipRedirectServer's list of suspended requests.  The SipRedirectServer
 *  guarantees to call the redirect plug-in's cancel() method once its
 *  services are no longer needed for that request.  The SipRedirectServer's
 *  cancel() method can free storage allocated for the request.
 *
 */

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

   /// Allow the redirectors to get the SipUserAgent
   SipUserAgent* sipUserAgent()
   {
      return mpSipUserAgent;
   }

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
   // forward redirected ACKs back to the proxy.
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
     static const char* AuthorityLevelPrefix;

     /**
      *  Fills in the supplied response with the common fields taken from the
      *  request and and the data contained in the supplied ErrorDescriptor
      */
     void buildResponseFromRequestAndErrorDescriptor( SipMessage& response,
                                                      const SipMessage& request,
                                                      const ErrorDescriptor& errorDescriptor );

     struct RedirectorDescriptor
     {
        bool      bActive;
        ssize_t   authorityLevel;
        UtlString name;
     };

     RedirectorDescriptor *mpConfiguredRedirectors;  // array containig info about the redirectors
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
