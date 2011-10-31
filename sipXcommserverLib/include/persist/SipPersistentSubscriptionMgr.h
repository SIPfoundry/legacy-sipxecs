//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipPersistentSubscriptionMgr_h_
#define _SipPersistentSubscriptionMgr_h_

#include <sipdb/SubscribeDB.h>
#include <net/SipDialogMgr.h>
#include <net/SipSubscriptionMgr.h>
#include <os/OsMsgQ.h>
#include <os/OsTimer.h>
#include <os/OsTime.h>
#include <os/OsServerTask.h>

class SipMessage;
class UtlString;

/*! Class for maintaining SUBSCRIBE dialog information in subscription server
 *  and storing the subscription states in the subscription.xml table.
 */
class SipPersistentSubscriptionMgr : public SipSubscriptionMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   //! Default constructor
   SipPersistentSubscriptionMgr(
      /// the "component" name for the table rows
      const UtlString& component,
      /// the AOR domain name
      const UtlString& domain,
      SubscribeDB& db);

   //! Destructor
   virtual
      ~SipPersistentSubscriptionMgr();

   //! Asks the SipPersistentSubscriptionMgr to initialize itself and sets the address 
   // of the queue to which to send resend messages.  The SipPersistentSubscriptionMgr
   // may not be used until initialize() returns.
   virtual void initialize(OsMsgQ* pMsgQ);
   
   //! Add/Update subscription for the given SUBSCRIBE request
   virtual UtlBoolean updateDialogInfo(const SipMessage& subscribeRequest,
                                       UtlString& resourceId,
                                       UtlString& eventTypeKey,
                                       UtlString& eventType,
                                       UtlString& subscribeDialogHandle,
                                       UtlBoolean& isNew,
                                       UtlBoolean& isExpired,
                                       SipMessage& subscribeResponse,
                                       SipSubscribeServerEventHandler& handler);

   //! Insert subscription dialog info without checking for the existence of the dialog
   /*! This method blindly inserts dialog information and should only be called from
    *  from the SipPersistentSubscriptionMgr.  It is intended to insert subscription
    *  information into memory from the IMDB.
    *
    *  NOTE: This method is not implemented in SipPersistentSubscriptionMgr.
    */
   virtual UtlBoolean insertDialogInfo(const SipMessage& subscribeRequest,
                                       const UtlString& resourceId,
                                       const UtlString& eventTypeKey,
                                       const UtlString& eventType,
                                       int expires,
                                       int notifyCSeq,
                                       int version,
                                       UtlString& subscribeDialogHandle,
                                       UtlBoolean& isNew);

   //! Set the subscription dialog information and cseq for the next NOTIFY request
   //  Also, update the database to show the to/from URIs as they appear in NOTIFYs.
   virtual UtlBoolean getNotifyDialogInfo(const UtlString& subscribeDialogHandle,
                                          SipMessage& notifyRequest,
                                          const char* subscriptionStateFormat,
                                          UtlString* resourceId = NULL,
                                          UtlString* eventTypeKey = NULL,
                                          UtlString* eventType = NULL,
                                          UtlString* acceptHeaderValue = NULL,
                                          bool* fullContent = NULL);

   //! End the dialog for the subscription indicated, by the dialog handle
   /*! Finds a matching dialog and expires the subscription if it has
    *  not already expired.
    *  \param dialogHandle - a fully established SIP dialog handle
    *  \param change - describes whether the subscription should
    *         end silently - subscriptionTerminatedSilently means that
    *         IMDB record of subscription should be retained.
    *  Returns TRUE if a matching dialog was found regardless of
    *  whether the subscription was already expired or not.
    */
   virtual UtlBoolean endSubscription(const UtlString& dialogHandle,
                                      enum SipSubscriptionMgr::subscriptionChange change);

   //! Remove old subscriptions that expired before given date
   virtual void removeOldSubscriptions(long oldEpochTimeSeconds);

   //! Set stored value for the next NOTIFY CSeq and version.
   virtual void setNextNotifyCSeq(const UtlString& dialogHandleString,
                                  int nextLocalCseq,
                                  int version);

   /** Update the saved value of the NOTIFY CSeq (now in notifyRequest) and
    *  the XML version (as specified).
    */
   virtual void updateVersion(SipMessage& notifyRequest,
                              int version,
                              const UtlString& eventTypeKey);

    /** get the next notify body "version" value that is allowed
     *  for a resource (as far as is known by this SipSubscriptionMgr).
     *  If no information is available, returns 0.
     */
    virtual int getNextAllowedVersion(const UtlString& resourceId);

protected:

private:

   //! "component" value to use in IMDB rows.
   UtlString mComponent;

   //! the AOR domain name
   UtlString mDomain;

   SubscribeDB& mDB;
   
   //! Copy constructor NOT ALLOWED
   SipPersistentSubscriptionMgr(const SipPersistentSubscriptionMgr& rSipPersistentSubscriptionMgr);

   //! Assignment operator NOT ALLOWED
   SipPersistentSubscriptionMgr& operator=(const SipPersistentSubscriptionMgr& rhs);

};

#endif  // _SipPersistentSubscriptionMgr_h_
