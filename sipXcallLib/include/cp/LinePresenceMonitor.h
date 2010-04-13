//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _LINEPRESENCEMONITOR_H_
#define _LINEPRESENCEMONITOR_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsBSem.h>
#include <os/OsEvent.h>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>
#include <net/StateChangeNotifier.h>
#include <net/SipDialogMonitor.h>
#include <net/SipRefreshManager.h>
#include <cp/LinePresenceBase.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * A LinePresenceMonitor object maintains subscriptions to obtain presence
 * and busy/not-busy information about users (URIs) and report it listeners.
 *
 * The object handles presence and dialog (busy/not-busy) information separately
 * but in similar ways.  Users of the object call it to register URIs for which
 * presence and/or dialog information should be obtained, and provide it with
 * LinePresenceBase* pointers through which to report the status information.
 *
 * The object maintains subscriptions for presence information itself.
 * Dialog information can be obtained locally (from a SipDialogMonitor
 * that the object creates) or remotely (by subscribing to a dialog event
 * server).  (Remote operation is not completely implemented.)
 *
 * This class is derived from StateChangeNotifier class so that it can pass
 * "this" to the SipDialogMonitor.  The SipDialogMonitor will then call
 * LinePresenceMonitor::setStatus() to report events.
 *
 * Typical use is:
 * @code
 * LinePresenceMonitor monitor(...);
 *
 * // Add URIs to watch for presence information.
 * // Each ACDAgent carries the URI to watch (via ::getUri()),
 * // and the callback for state changes (via ::updateState()).
 * ACDAgent agent1(...);
 * monitor.subscribePresence(&agent1);
 * ACDAgent agent2(...);
 * monitor.subscribePresence(&agent2);
 *
 * // Remove URIs from being watched.
 * monitor.unsubscribePresence(&agent1);
 * monitor.unsubscribePresence(&agent2);
 * // Now agent1 and agent2 can be deleted.
 * @endcode
 */
class LinePresenceMonitor : public StateChangeNotifier, public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Constructor
   LinePresenceMonitor(int userAgentPort,       ///< user agent port
                       UtlString& domainName,   ///< sipX domain name
                       UtlString& groupName,    /**< name of the group to be monitored
                                                 *   (not implemented)
                                                 */
                       bool local,              /**< option for using local or remote monitor
                                                 *   (remote use not implemented)
                                                 */
                       Url& remoteServerUrl,    /**< remote monitor server url
                                                 *   (not implemented)
                                                 */
                       Url& presenceServerUrl); ///< presence server url

   /// Destructor
   virtual ~LinePresenceMonitor();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   /** Implement StateChangeNotifier::setStatus.
    *
    *  A LinePresenceMonitor uses this method by passing "this" to
    *  to the SipDialogMonitor mpDialogMonitor.
    *  mpDialogMonitor will call [LinePresenceMonitor]->setStatus([status])
    *  to report changes in busy/not-busy status.
    *
    *  Because setStatus can be called asynchronously (including when other
    *  LinePresenceMonitor methods are executing), setStatus is asynchronous --
    *  it posts a message to the LinePresenceMonitor queue to have
    *  setStatusInternal update the status variables.
    */
   bool setStatus(const Url& aor, const StateChangeNotifier::Status value);

   /** Instruct the LinePresenceMonitor to monitor the dialog status
    *  (busy/not-busy) of the URI in line->getUri().
    *  Status changes are reported with line->updateState().
    *  "*line" must not be deleted until "unsubscribeDialog(line)" has
    *  been called the event signalled
    */
   OsStatus subscribeDialog(LinePresenceBase* line);

   /** Instruct the LinePresenceMonitor to cease monitoring the dialog status
    *  of a URI.
    *  Once the event has been signaled, "*line" may be deleted.
    */
   OsStatus unsubscribeDialog(LinePresenceBase* line, OsEvent* e = NULL);

   /** Subscribe to the dialog status of the lines on a list.
    *  The line objects are subject to the same restrictions as from
    *  subscribeDialog(LinePresenceBase*).
    */
   OsStatus subscribeDialog(UtlSList& list);

   /** Unsubscribe to the dialog status of the lines on a list.
    *  Once the event has been signaled, the lines in the list may be deleted.
    */
   OsStatus unsubscribeDialog(UtlSList& list, OsEvent* e = NULL);

   /** Instruct the LinePresenceMonitor to monitor the presence status
    *  of the URI in line->getUri().
    *  Status changes are reported with line->updateState().
    *  "*line" must not be deleted until "unsubscribeDialog(line)" has
    *  been called and the event signalled.
    */
   OsStatus subscribePresence(LinePresenceBase* line);

   /** Instruct the LinePresenceMonitor to cease monitoring the presence status
    *  of a URI.
    *  Once the event has been signalled, "*line" may be deleted.
    */
   OsStatus unsubscribePresence(LinePresenceBase* line, OsEvent* e = NULL);

   /** Subscribe to the presence status of the lines on a list
    *  The line objects are subject to the same restrictions as from
    *  subscribePresence(LinePresenceBase*).
    */
   OsStatus subscribePresence(UtlSList& list);

   /** Unsubscribe to the presence status of the lines on a list
    *  Once the event has been signaled, the lines in the list may be deleted.
    */
   OsStatus unsubscribePresence(UtlSList& list, OsEvent* e = NULL);

/* ============================ INQUIRY =================================== */

   /** Check the status of the monitor.  Returns true if the monitor is
    *  initialized successfully or false if any problems are detected
    *  (e.g. SipUserAgent failed a port binding).
    */
   UtlBoolean isOk() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   static void subscriptionStateCallback(SipSubscribeClient::SubscriptionState newState,
                                         const char* earlyDialogHandle,
                                         const char* dialogHandle,
                                         void* applicationData,
                                         int responseCode,
                                         const char* responseText,
                                         long expiration,
                                         const SipMessage* subscribeResponse);

   static bool notifyEventCallback(const char* earlyDialogHandle,
                                   const char* dialogHandle,
                                   void* applicationData,
                                   const SipMessage* notifyRequest);

   void handleNotifyMessage(const SipMessage* notifyMessage);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   // The SipUserAgent used to issue SUBSCRIBEs for presence events.
   SipUserAgent* mpUserAgent;
   /** mGroupName was designed to support naming groups of URIs for monitoring,
    *  but that facility has not been implemented.
    */
   UtlString mGroupName;
   /** TRUE if the LinePresenceMonitor should maintain its own SUBSCRIBEs for
    *  dialog events.  FALSE means to subscribe to a Dialog Event Server
    *  for dialog events.
    */
   bool mLocal;
   UtlString mDomainName;
   // Contact addr for any SUBSCRIBEs sent.
   UtlString mContact;

   /** If mLocal is TRUE, a pointer to the SipDialogMonitor created to monitor
    *  dialog event status.
    */
   SipDialogMonitor* mpDialogMonitor;

   /** A SipDialogManager, SipefreshMgr and SipSubscribeClient to
    *  maintain SUBSCRIBEs for presence events.
    */
   SipDialogMgr mDialogManager;
   SipRefreshManager* mpRefreshMgr;
   SipSubscribeClient* mpSipSubscribeClient;

   // URI of the "remote" DialogEventServer.
   Url mRemoteServer;
   // URI of the Presence Server.
   UtlString mPresenceServer;

   /** Maps URIs for which dialog information is to be kept to
    *  LinePresenceBase*'s with which to report status changes.
    */
   UtlHashMap mDialogSubscribeList;
   /** Maps URIs for which presence information is to be kept to
    *  LinePresenceBase*'s with which to report status changes.
    */
   UtlHashMap mPresenceSubscribeList;
   /** Maps URIs for which presence information is to be kept to
    *  the early subscription handles for their presence subscriptions.
    */
   UtlHashMap mDialogHandleList;

   /// Semaphore to serialize operations on the object.
   OsBSem mLock;

   /// Handle incoming IPC messages
   UtlBoolean handleMessage(OsMsg& rMessage);

   /// Subscribe the dialog on a specific line in the list
   OsStatus subscribeDialogMessage(LinePresenceBase* line);

   /// Unsubscribe the dialog on a specific line from the list
   OsStatus unsubscribeDialogMessage(LinePresenceBase* line);

   /// Subscribe the presence on a specific line in the list
   OsStatus subscribePresenceMessage(LinePresenceBase* line);

   /// Unsubscribe the presence on a specific line from the list
   OsStatus unsubscribePresenceMessage(LinePresenceBase* line);

   /** Perform the work of updating the status variables.
    *  This method does not take mLlock, but assumes that its caller
    *  holds mLock.
    *  This method deletes *contact.
    */
   OsStatus setStatusMessage(const UtlString* contact,
                             const StateChangeNotifier::Status value);

   /// Disabled copy constructor
   LinePresenceMonitor(const LinePresenceMonitor& rLinePresenceMonitor);

   /// Disabled assignment operator
   LinePresenceMonitor& operator=(const LinePresenceMonitor& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _LINEPRESENCEMONITOR_H_
