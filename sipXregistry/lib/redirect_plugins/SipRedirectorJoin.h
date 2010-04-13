//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORJOIN_H
#define SIPREDIRECTORJOIN_H

// SYSTEM INCLUDES
//#include <sys/time.h>

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"
#include "os/OsServerTask.h"
#include "os/OsTimer.h"
#include "xmlparser/tinyxml.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRedirectorJoinTask;

/**
 * SipRedirectorJoin is a singleton class whose object implements
 * the redirections needed for call barge-in.
 *
 * The redirections implemented is:
 *
 * [call join][extension]
 *    is mapped into a selected early dialog discovered by querying [extension]
 */

class SipRedirectorJoin : public RedirectPlugin
{
  public:

   explicit SipRedirectorJoin(const UtlString& instanceName);

   ~SipRedirectorJoin();

   /** SipRedirectorJoin has several configuration parameters, listed at the
    *  top of SipRedirectorJoin.cpp.
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

   // Get the top-level text content of an XML element.
   static void textContentShallow(UtlString& string,
                                  TiXmlElement *element);
   // Get the complete text content of an XML element (including sub-elements).
   static void textContentDeep(UtlString& string,
                               TiXmlElement *element);
   // Service function for textContentDeep.
   static void textContentDeepRecursive(UtlString& string,
                                        TiXmlElement *element);

  protected:

   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;

   /** OS_SUCCESS if this redirector is configured to do any work,
    * and OS_FAILED if not.
    */
   OsStatus mRedirectorActive;

   // The SIP user agent to send SUBSCRIBEs and receive NOTIFYs.
   SipUserAgent* mpSipUserAgent;

   // The OsTask that processes NOTIFYs.
   SipRedirectorJoinTask* mTask;

   // The feature code (SIP user prefix) for call join.
   UtlString mCallJoinCode;

   // The two SIP users that are excluded from being considered
   // join requests, so we can use them for other features.
   UtlString mExcludedUser1;
   UtlString mExcludedUser2;

   // The SIP domain we are operating in.
   UtlString mDomain;

   // Time to wait for NOTIFYs, in seconds and microseconds.
   int mWaitSecs;
   int mWaitUSecs;

   // Counter for CSeq for SUBSCRIBES.
   unsigned int mCSeq;

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

class SipRedirectorJoinTask : public OsServerTask
{
  public:

   SipRedirectorJoinTask(
      SipUserAgent* pSipUserAgent,
      int redirectorNo);

   ~SipRedirectorJoinTask();

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
class SipRedirectorJoinNotification : public OsNotification
{
  public:

   SipRedirectorJoinNotification(
      RedirectPlugin::RequestSeqNo requestSeqNo,
      int redirectorNo);

   OsStatus signal(const intptr_t eventData);

  private:

   RedirectPlugin::RequestSeqNo mRequestSeqNo;
   int mRedirectorNo;
};

/**
 * Private storage for call-join suspensions.
 */
class SipRedirectorPrivateStorageJoin : public SipRedirectorPrivateStorage
{
   friend class SipRedirectorJoin;
   friend class SipRedirectorJoinTask;

  public:

   SipRedirectorPrivateStorageJoin(RedirectPlugin::RequestSeqNo requestSeqNo,
                                     int redirectorNo);

   virtual ~SipRedirectorPrivateStorageJoin();

   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE ;    /**< Class type used for runtime checking */

  protected:

   // CallId of the SUBSCRIBE we sent.
   UtlString mSubscribeCallId;

   // State filtering criteria.
   SipRedirectorJoin::State mStateFilter;

  private:

   // Notification.
   SipRedirectorJoinNotification mNotification;

   // Timer.
   OsTimer mTimer;

   // When the SUBSCRIBE was sent.
   OsTime mSubscribeSendTime;

   // Process a NOTIFY body and record the calls as appropriate.
   void processNotify(const char* body);
   // Not quite the same as SipRedirectorPickUp::processNotifyDialogElement.
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
   // URI at which to contact the local end
   UtlString mTargetDialogLocalURI;
   // Local Identity
   UtlString mTargetDialogLocalIdentity;
};

#endif // SIPREDIRECTORJOIN_H
