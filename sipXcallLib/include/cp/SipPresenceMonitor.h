//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SIPPRESENCEMONITOR_H_
#define _SIPPRESENCEMONITOR_H_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsBSem.h>
#include <os/OsConfigDb.h>
#include <os/OsServerTask.h>
#include <os/OsTime.h>
#include <os/OsTimer.h>
#include <net/StateChangeNotifier.h>
#include <cp/PresenceDialInServer.h>
#include <net/SipUserAgent.h>
#include <net/SipDialogMgr.h>
#include <net/SipSubscriptionMgr.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipSubscribeServer.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipPresenceEvent.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlSList.h>
#include <utl/UtlString.h>
#include <cp/CallManager.h>
#include <cp/XmlRpcSignIn.h>
#include <net/SdpCodecFactory.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * A SipPresenceMonitor keeps track of the SIP user agents' presence
 * status. The SipPresenceMonitor attaches a CallManager and
 * PresenceDialInServer to the SipUserAgent, so that it can receive
 * and interpret sign-in/sign-out calls.  The SipPresenceMonitor
 * operates an XML listener (XmlRpcSignin) to receive XML RPC calls to
 * sign-in/sign-out.  Furthermore, the caller can register a
 * StateChangeNotifier with SipPresenceMonitor so that state changes
 * will be delivered to the StateChangeNotifier.
 *
 * If toBePblished is TRUE:  subscriptionMgr must point to a
 * SipSubscriptionMgr.  Along with a SipSubscribeServer that
 * SipPresenceMonitor creates, these are used to service subscriptions
 * to the presence information.
 */

/// Task that periodically writes the presence statuses to a disk file.
class SipPresenceMonitorPersistenceTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   //! Default constructor
   SipPresenceMonitorPersistenceTask(SipPresenceMonitor* presenceMonitor);

   //! Destructor
   virtual
      ~SipPresenceMonitorPersistenceTask();

/* ============================ MANIPULATORS ============================== */

   UtlBoolean handleMessage(OsMsg& rMsg);
   /// Method to process messages which get queued for this OsServerTask.

   void stop();
   /// Stop the task properly.

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   //! Pointer to the SipPresenceMonitor that we are to store to disk.
   SipPresenceMonitor* mSipPresenceMonitor;

   //! Copy constructor NOT ALLOWED
   SipPresenceMonitorPersistenceTask(const SipPresenceMonitorPersistenceTask& rSipPresenceMonitorPersistenceTask);

   //! Assignment operator NOT ALLOWED
   SipPresenceMonitorPersistenceTask& operator=(const SipPresenceMonitorPersistenceTask& rhs);
};


class SipPresenceMonitor : public StateChangeNotifier
{
   friend class SipPresenceMonitorPersistenceTask;
   friend class PresenceDefaultConstructor;

  public:

   SipPresenceMonitor(SipUserAgent* userAgent,   /**<
                                                 * SipUserAgent for sending out
                                                 * SUBSCRIBEs and receiving NOTIFYs
                                                 */
                      SipSubscriptionMgr* subscriptionMgr,
                                                ///< subscription manager to record subscriptions
                      UtlString& domainName,    ///< sipX domain name
                      int hostPort,             ///< Host port
                      OsConfigDb*configFile,    ///< configuration
                      bool toBePublished,       ///< option to publish for other subscriptions
                      const char* persistentFile); ///< name of file to persist into, or NULL

   virtual ~SipPresenceMonitor();

   /// Add an extension to a group to be monitored
   bool addExtension(UtlString& groupName, Url& contactUrl);

   /// Remove an extension from a group to be monitored
   bool removeExtension(UtlString& groupName, Url& contactUrl);

   /// Register a StateChangeNotifier
   void addStateChangeNotifier(const char* fileUrl, StateChangeNotifier* notifier);

   /// Unregister a StateChangeNotifier
   void removeStateChangeNotifier(const char* fileUrl);

   /// Set the status value for the URI 'aor'.
   virtual bool setStatus(const Url& aor, const Status value);
   // Returns TRUE if the requested state is different from the current state.

   /// Get the state of the contact
   void getState(const Url& aor, UtlString& status);

  protected:

   /// Add the contact and presence event to the subscribe list
   bool addPresenceEvent(UtlString& contact, SipPresenceEvent* presenceEvent);
   // Returns TRUE if the requested state is different from the current state.

   /// Publish the presence event package to the resource list
   void publishContent(UtlString& contact, SipPresenceEvent* presenceEvent);

   /// Send the state change to the notifier
   void notifyStateChange(UtlString& contact, SipPresenceEvent* presenceEvent);
   // Caller must hold mLock.

   //! Construct a tuple id from a presence resource name.
   static void makeId(UtlString& id,             ///< output: tuple id
                      const UtlString& resource  ///< resource URI
      );

   /// Read the presence events from the persistent file.
   void readPersistentFile();

   /// Write the presence events to the persistent file.
   void writePersistentFile();

  private:

   CallManager* mpCallManager;
   SdpCodecFactory mCodecFactory;
   PresenceDialInServer* mpDialInServer;
   SipUserAgent* mpUserAgent;
   UtlString mDomainName;
   UtlString mHostAndPort;
   bool mToBePublished;

   OsBSem mLock;

   SipSubscriptionMgr* mpSubscriptionMgr;
   SipSubscribeServerEventHandler mPolicyHolder;
   SipPublishContentMgr mSipPublishContentMgr;
   SipSubscribeServer* mpSubscribeServer;

   XmlRpcSignIn* mpXmlRpcSignIn;

   UtlHashMap mMonitoredLists;
   UtlHashMap mPresenceEventList;
   UtlHashMap mStateChangeNotifiers;

   /// Disabled copy constructor
   SipPresenceMonitor(const SipPresenceMonitor& rSipPresenceMonitor);

   /// Disabled assignment operator
   SipPresenceMonitor& operator=(const SipPresenceMonitor& rhs);

   /// Path name of the persistence file, or the null string for no persistence.
   UtlString mPersistentFile;

   //! Timer for flushing changes to disk.
   /** When mPresenceEventList is dirty (changes have not been written to disk),
    *  this timer is running; when it is clean, this timer is stopped.
    *  (This works because the OsTimer start and stop methods are fully
    *  interlocked, and report whether the timer was previously running.)
    */
   OsTimer mPersistenceTimer;

   //! Time interval to save changes to disk.
   static const OsTime sPersistInterval;

   //! The 'type' attribute of the top-level 'items' element.
   static const UtlString sType;

   //! The XML namespace of the top-level 'items' element.
   static const UtlString sXmlNamespace;

   //! Task to save changes to disk.
   SipPresenceMonitorPersistenceTask mPersistTask;
};

#endif // _SIPPRESENCEMONITOR_H_
