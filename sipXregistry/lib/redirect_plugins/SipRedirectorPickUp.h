//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORPICKUP_H
#define SIPREDIRECTORPICKUP_H

// SYSTEM INCLUDES
//#include <sys/time.h>

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"
#include "os/OsServerTask.h"
#include "os/OsTimer.h"
#include "xmlparser/tinyxml.h"
#include "filereader/OrbitFileReader.h"

// DEFINES
#define ALL_CREDENTIALS_USER "~~sp~allcredentials"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRedirectorPickUpTask;

/**
 * SipRedirectorPickUp is a singleton class whose object implements
 * the redirections needed for call pick-up.
 *
 * There are several redirections implemented:
 *
 * [directed call pick-up][extension]
 *    is mapped into a selected early dialog discovered by querying [extension]
 * [global call pick-up]
 *    is mapped into a selected early dialog discovered by querying the special
 *    SIP user "~~sp~allcredentials"
 * ~~sp~allcredentials (ALL_CREDENTIALS_USER)
 *    is mapped into all extensions mentioned in the credential table
 * [call retrieve][extension]
 *    is mapped into a selected confirmeddialog discovered by querying
 *    [extension].  But [extension] must be listed as a call park orbit in
 *    orbits.xml.
 * [extension]
 *    When [extension] is listed in orbits.xml, redirects to
 *    "[extension]@SIP_REGISTRAR_ORBIT_SERVER".
 *
 * More information on their processing and usage can be found in
 * ../doc/Redirection.txt.
 */

class SipRedirectorPickUp : public RedirectPlugin
{
  public:

   explicit SipRedirectorPickUp(const UtlString& instanceName);

   ~SipRedirectorPickUp();

   /** SipRedirectorPickUp has many configuration parameters, listed at the
    *  top of SipRedirectorPickUp.cpp.
    */
   virtual void readConfig(OsConfigDb& configDb);

   virtual OsStatus initialize(OsConfigDb& configDb,
                               int redirectorNo,
                               const UtlString& localDomainHost);

   virtual void finalize();

   virtual RedirectPlugin::LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      ContactList& contactList,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage,
      ErrorDescriptor& errorDescriptor);

   virtual const UtlString& name( void ) const;

   // Enum for values that describe the states of dialogs, and also
   // describe filtering criteria for states of dialogs.
   typedef enum {
      // State is unknown.  (Used only as a description.)
      stateUnknown,
      // Matches any state.  (Used only as a matching criterion.)
      stateDontCare,
      // Any other state value not classified.  (Used only as a description.)
      stateOther,
      // "early"
      stateEarly,
      // "confirmed"
      stateConfirmed
   } State;

  protected:

   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;

   // Derivative strings for arguments to addContact().
   UtlString mLogNameGlobalPickUp;
   UtlString mLogNameOrbit;
   UtlString mLogNamePickUp;
   UtlString mLogNameRetrieve;

   /** OS_SUCCESS if this redirector is configured to do any work,
    * and OS_FAILED if not.
    */
   OsStatus mRedirectorActive;

   // The SIP user agent to send SUBSCRIBEs and receive NOTIFYs.
   SipUserAgent* mpSipUserAgent;

   // The OsTask that processes NOTIFYs.
   SipRedirectorPickUpTask* mTask;

   // The feature code (SIP user prefix) for directed pick-up.
   UtlString mCallPickUpCode;

   // The two SIP users that are excluded from being considered
   // pick-up requests, so we can use them for other features.
   UtlString mExcludedUser1;
   UtlString mExcludedUser2;

   // The SIP username for global pick-up.
   UtlString mGlobalPickUpCode;

   // The SIP username prefix for call retrieval.
   UtlString mCallRetrieveCode;

   // The SIP domain of the park server.
   UtlString mParkServerDomain;

   // OrbitFileReader object to read and store information in orbits.xml.
   OrbitFileReader mOrbitFileReader;

   // The SIP domain we are operating in.
   UtlString mDomain;

   // Time to wait for NOTIFYs, in seconds and microseconds.
   int mWaitSecs;
   int mWaitUSecs;

   // Counter for CSeq for SUBSCRIBES.
   unsigned int mCSeq;

   // Switch for "no early-only" workaround.
   UtlBoolean mNoEarlyOnly;

   // Switch for "reversed Replaces" workaround.
   UtlBoolean mReversedReplaces;

   // Switch for the "1 second subscription" workaround.
   UtlBoolean mOneSecondSubscription;

   // Support functions.
   RedirectPlugin::LookUpStatus lookUpDialog(
      const UtlString& requestString,
      const UtlString& incomingCallId,
      ContactList& contactList,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage,
      const char* subscribeUser,
      State stateFilter);

   SipLineMgr* addCredentials(UtlString domain, UtlString realm);
};

/**
 * Task to receive and act on NOTIFYs.
 */

class SipRedirectorPickUpTask : public OsServerTask
{
  public:

   SipRedirectorPickUpTask(
      SipUserAgent* pSipUserAgent,
      int redirectorNo);

   ~SipRedirectorPickUpTask();

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

  protected:

  private:

   // The SIP user agent to send SUBSCRIBEs and receive NOTIFYs.
   SipUserAgent* mpSipUserAgent;

   // Our redirector number.
   int mRedirectorNo;
};

/**
 * Notifications for when our timers fire.
 */
class SipRedirectorPickUpNotification : public OsNotification
{
  public:

   SipRedirectorPickUpNotification(
      RedirectPlugin::RequestSeqNo requestSeqNo,
      int redirectorNo);

   OsStatus signal(const intptr_t eventData);

  private:

   RedirectPlugin::RequestSeqNo mRequestSeqNo;
   int mRedirectorNo;
};

/**
 * Private storage for pick-up suspensions.
 */
class SipRedirectorPrivateStoragePickUp : public SipRedirectorPrivateStorage
{
   friend class SipRedirectorPickUp;
   friend class SipRedirectorPickUpTask;

  public:

   SipRedirectorPrivateStoragePickUp(RedirectPlugin::RequestSeqNo requestSeqNo,
                                     int redirectorNo);

   virtual ~SipRedirectorPrivateStoragePickUp();

   virtual UtlContainableType getContainableType() const;

  protected:

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   // CallId of the SUBSCRIBE we sent.
   UtlString mSubscribeCallId;

   // State filtering criteria.
   SipRedirectorPickUp::State mStateFilter;

  private:

   // Notification.
   SipRedirectorPickUpNotification mNotification;

   // Timer.
   OsTimer mTimer;

   // When the SUBSCRIBE was sent.
   OsTime mSubscribeSendTime;

   // Process a NOTIFY body and record the calls as appropriate.
   void processNotify(const char* body);
   void processNotifyDialogElement(TiXmlElement* dialog);
   void processNotifyLocalRemoteElement(TiXmlElement* element,
                                        UtlString& identity,
                                        UtlString& target);

   // Information about the best dialog to pick up that we have seen so far.
   // Call-Id
   UtlString mTargetDialogCallId;
   // Duration, or 0 if none was given for the dialog, or the special value
   // TargetDialogDurationAbsent if no dialog is recorded.
   // If no <duration> is specified in a dialog event notice (and many
   // event notices do not contain it), use 0.
   // Since we select the call with the largest duration, this puts
   // phones that do not provide <duration> at a disadvantage to get
   // their calls picked up.
   int mTargetDialogDuration;
   // Note that the special value must be less than any legitimate duration.
   static const int TargetDialogDurationAbsent;
   // Local tag (from the point of view of the UA we are picking up from)
   UtlString mTargetDialogLocalTag;
   // Remote tag
   UtlString mTargetDialogRemoteTag;
   // URI at which to contact the remote end.
   UtlString mTargetDialogRemoteURI;
   // Remote Identity.
   UtlString mTargetDialogRemoteIdentity;
   // URI at which to contact the local end
   UtlString mTargetDialogLocalURI;
   // Local Identity
   UtlString mTargetDialogLocalIdentity;
};

#endif // SIPREDIRECTORPICKUP_H
