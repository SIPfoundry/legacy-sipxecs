//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <stdlib.h>

// APPLICATION INCLUDES

#include <persist/SipPersistentSubscriptionMgr.h>
#include <utl/UtlString.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlSListIterator.h>
#include <utl/XmlContent.h>
#include <os/OsLogger.h>
#include <os/OsTimer.h>
#include <os/OsDateTime.h>
#include <os/OsEventMsg.h>
#include <net/SipMessage.h>
#include <net/SipDialog.h>
#include <sipdb/ResultSet.h>
#include <assert.h>

// Constructor
SipPersistentSubscriptionMgr::SipPersistentSubscriptionMgr(
   const UtlString& component,
   const UtlString& domain,
   SubscribeDB& db) :
   mComponent(component),
   mDomain(domain),
   mDB(db)
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr:: "
                 "mComponent = '%s', mDomain = '%s'",
                 mComponent.data(), mDomain.data());
}

// Destructor
SipPersistentSubscriptionMgr::~SipPersistentSubscriptionMgr()
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::~");
}

/* ============================ MANIPULATORS ============================== */
void SipPersistentSubscriptionMgr::initialize(OsMsgQ* pMsgQ)
{
   // initialize the base class
   SipSubscriptionMgr::initialize(pMsgQ);

   // Read the subscription table and initialize the SipSubscriptionMgr.

   unsigned long now = OsDateTime::getSecsSinceEpoch();
   SubscribeDB::Subscriptions subscriptions;
   mDB.getAll(subscriptions);

   for (SubscribeDB::Subscriptions::iterator iter = subscriptions.begin();
       iter != subscriptions.end(); iter++)
   {
      Subscription& row = *iter;

      if (row.component() == mComponent.str() && row.expires() - now >= 0)
      {
         Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                       "SipPersistentSubscriptionMgr:: "
                       "loading row");

         // Use SipSubscriptionMgr to update the in-memory data.

         // Construct a fake SUBSCRIBE request to carry most of the data
         // that updateDialogInfo needs.
         SipMessage subscribeRequest;
         Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                       "SipPersistentSubscriptionMgr:: expires = %d, now = %d",
                       row.expires(), (int) now);
         
         subscribeRequest.setSubscribeData(row.uri().c_str(),
                                           row.fromUri().c_str(),
                                           row.toUri().c_str(),
                                           row.callId().c_str(),
                                           row.subscribeCseq(),
                                           row.eventTypeKey().c_str(),
                                           row.accept().c_str(),
                                           row.id().c_str(),
                                           row.contact().c_str(),
                                           NULL,
                                           row.expires() - now);
         // Install the saved Route as a set of Record-Route headers in the
         // SUBSCRIBE, so that insertDialogInfo will find and record the route.
         Url route_url;
         UtlString route_url_string;
         UtlString route_string(row.recordRoute().c_str());
         UtlString remainder_string;
         int route_index;
         for (route_index = 0;
              !route_string.isNull() &&
                 route_url.fromString(route_string, Url::NameAddr, &remainder_string);
              route_string = remainder_string, route_index++)
         {
            route_url.toString(route_url_string);
            subscribeRequest.setRecordRouteField(route_url_string.data(), route_index);
         }
         if (Os::Logger::instance().willLog(FAC_SIP, PRI_DEBUG))
         {
            UtlString m, d;
            ssize_t l;
            subscribeRequest.getBytes(&m, &l, FALSE);
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipPersistentSubscriptionMgr:: subscribeRequest = '%s'",
                          m.data());
         }

         // Variables to hold the output of insertDialogInfo.
         UtlString subscribeDialogHandle;
         UtlBoolean isNew;
         SipSubscriptionMgr::insertDialogInfo(subscribeRequest,
                                                 // *keyp is the resource that
                                                 // is subscribed to.
                                                 row.key().c_str(),
                                                 row.eventTypeKey().c_str(),
                                                 row.eventType().c_str(),
                                                 row.expires(),
                                                 row.notifyCseq(),
                                                 row.version(),
                                                 subscribeDialogHandle,
                                                 isNew);
		// Set the next NOTIFY CSeq value.
		// (The data in IMDB has already been set.)
		SipSubscriptionMgr::setNextNotifyCSeq(subscribeDialogHandle,
											  row.notifyCseq(),
											  row.version());
      }
   }
}

UtlBoolean SipPersistentSubscriptionMgr::updateDialogInfo(
   const SipMessage& subscribeRequest,
   UtlString& resourceId,
   UtlString& eventTypeKey,
   UtlString& eventType,
   UtlString& subscribeDialogHandle,
   UtlBoolean& isNew,
   UtlBoolean& isSubscriptionExpired,
   SipMessage& subscribeResponse,
   SipSubscribeServerEventHandler& handler)
{
   UtlBoolean ret;

   // Call SipSubscriptionMgr to update the in-memory data.
   ret = SipSubscriptionMgr::updateDialogInfo(subscribeRequest,
                                              resourceId,
                                              eventTypeKey,
                                              eventType,
                                              subscribeDialogHandle,
                                              isNew,
                                              isSubscriptionExpired,
                                              subscribeResponse,
                                              handler);

   // If that succeeded, update the IMDB.
   if (ret)
   {
      UtlString requestUri;
      UtlString callId;
      UtlString contactEntry;
      UtlString to;
      UtlString from;
      UtlString route;
      UtlString accept;

      subscribeRequest.getRequestUri(&requestUri);
      subscribeRequest.getCallIdField(&callId);
      subscribeRequest.getContactEntry(0, &contactEntry);
      subscribeRequest.getToField(&to);
      subscribeRequest.getFromField(&from);
      subscribeRequest.buildRouteField(&route);
      accept.append(subscribeRequest.getHeaderValue(0, SIP_ACCEPT_FIELD));

      int expires = 0;
      subscribeResponse.getExpiresField(&expires);
      expires += OsDateTime::getSecsSinceEpoch();

      int subscribeCseq;
      UtlString subscribeCseqMethod;
      subscribeRequest.getCSeqField(&subscribeCseq, &subscribeCseqMethod);

      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "SipPersistentSubscriptionMgr::updateDialogInfo "
                    "mComponent = '%s', requestUri = '%s', callId = '%s', contactEntry = '%s', expires = %d, "
                    "to = '%s', from = '%s', eventTypeKey = '%s', eventType = '%s', key = '%s', route = '%s', accept = '%s'",
                    mComponent.data(), requestUri.data(),
                    callId.data(), contactEntry.data(), expires,
                    to.data(), from.data(), eventTypeKey.data(),
                    eventType.data(), resourceId.data(), route.data(),
                    accept.data());
      mDB.upsert(mComponent, requestUri, callId, contactEntry, expires,
				subscribeCseq, eventTypeKey, eventType, "", to, from,
				resourceId, route, 
        isNew,  // set the current notify cseq.  true(1) for new dialogs and false(0) for refresh
        accept, 0);
   }

   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::updateDialogInfo "
                 "subscribeDialogHandle = '%s', "
                 "ret = %d, isNew = %d, isSubscriptionExpired = %d, "
                 "resourceId = '%s', eventTypeKey = '%s', eventType = '%s'",
                 subscribeDialogHandle.data(),
                 ret, isNew, isSubscriptionExpired,
                 resourceId.data(), eventTypeKey.data(), eventType.data());

   return ret;
}

UtlBoolean SipPersistentSubscriptionMgr::insertDialogInfo(
   const SipMessage& subscribeRequest,
   const UtlString& resourceId,
   const UtlString& eventTypeKey,
   const UtlString& eventType,
   int expires,
   int notifyCSeq,
   int version,
   UtlString& subscribeDialogHandle,
   UtlBoolean& isNew)
{
   // This method is defined on the SipSubscriptionMgr to populate state from
   // the IMDB -- it should never be called on the SipPersistentSubscriptionMgr.
   assert(FALSE);
}

// Set the subscription dialog information and cseq for the next NOTIFY request
UtlBoolean SipPersistentSubscriptionMgr::getNotifyDialogInfo(
   const UtlString& subscribeDialogHandle,
   SipMessage& notifyRequest,
   const char* subscriptionStateFormat,
   UtlString* resourceId,
   UtlString* eventTypeKey,
   UtlString* eventType,
   UtlString* acceptHeaderValue,
   bool* fullContent)
{
   UtlBoolean ret;

   // Call SipSubscriptionMgr to update the in-memory data.
   ret = SipSubscriptionMgr::getNotifyDialogInfo(subscribeDialogHandle,
                                                 notifyRequest,
                                                 subscriptionStateFormat,
                                                 resourceId,
                                                 eventTypeKey,
                                                 eventType,
                                                 acceptHeaderValue,
                                                 fullContent);

   // If that succeeded, update the IMDB to show the to/from URIs as
   // they appear in the NOTIFY, esp. including the to-tag.
   if (ret)
   {
      // Extract the To and From headers from the NOTIFY constructed
      // by SipSubscriptionMgr.
      UtlString from, to;
      notifyRequest.getFromField(&from);
      notifyRequest.getToField(&to);

      UtlString callId, localTag, remoteTag;
      SipDialog::parseHandle(subscribeDialogHandle.data(),
                             callId, localTag, remoteTag);
      // It's not clear why the localTag is the from-tag of the original SUBSCRIBE,
      // but it is, and that's what we need to feed to updateFromAndTo.
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "SipPersistentSubscriptionMgr::getNotifyDialogInfo "
                    "subscribeDialogHandle = '%s', localTag = '%s', from = '%s', to = '%s'",
                    subscribeDialogHandle.data(), localTag.data(),
                    from.data(), to.data());
      // Update the to-tag of the recorded To field for the subscriptions.
      // (This is rather a crock.  The subscription DB indexes subscriptions
      // by call-id, To, and From, whereas it should be using only
      // call-id, to-tag, and from-tag.  The rest of the To and From fields
      // might change -- not only due to the remote end, but trivial
      // reformattings due to translating strings to/from URIs.  But fixing
      // that will have to wait until the SipSubscriptionMgr machinery is
      // overhauled.)
      // At this point, we reverse the roles of "from" and "to",
      // because updateFromAndTo uses these words in relationship to
      // the SUBSCRIBE, whereas getNotifyDialogInfo uses these words
      // in relationship to the NOTIFY, which is the reverse order.
      mDB.updateToTag(callId, localTag, remoteTag);
   }

   // The NOTIFY CSeq and XML version will be saved when our caller
   // calls updateVersion().

   return ret;
}

UtlBoolean SipPersistentSubscriptionMgr::endSubscription(const UtlString& dialogHandle,
                                                         enum SipSubscriptionMgr::subscriptionChange change)
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::endSubscription "
                 "dialogHandle = '%s', change = %d",
                 dialogHandle.data(), change);

   UtlBoolean ret;

   // Call SipSubscriptionMgr to update the in-memory data.
   ret = SipSubscriptionMgr::endSubscription(dialogHandle, change);

   // If that succeeded, and change != silent, update the IMDB.
   if (ret && change != SipSubscriptionMgr::subscriptionTerminatedSilently)
   {
      // Delete the subscription from the IMDB.
      UtlString callId, localTag, remoteTag;
      SipDialog::parseHandle(dialogHandle.data(),
                             callId, localTag, remoteTag);
      UtlString from, to;
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "SipPersistentSubscriptionMgr::endSubscription callId = '%s', localTag = '%s', remoteTag = '%s'",
                    callId.data(), localTag.data(), remoteTag.data());

      if (mDB.findFromAndTo(callId, localTag, remoteTag,
                                                 from, to))
      {
         // We use the largest possible CSeq number here, as we assume
         // that no more than one subscription is established in a given dialog.
         // That assumption is incorrect, but the rest of the SipSubscriptionMgr
         // machinery makes that assumption, and does not provide us with the CSeq
         // value, nor with the event type and id, which are necessary to uniquely
         // identify a subscription.  (See RFC 3265, section 7.2.1.)
         mDB.remove(mComponent, to, from, callId, 0x7FFFFFFF);
      }
      else
      {
         Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                       "SipPersistentSubscriptionMgr::endSubscription "
                       "Cannot find subscription for dialog handle '%s'",
                       dialogHandle.data());
      }
   }

   return ret;
}

void SipPersistentSubscriptionMgr::removeOldSubscriptions(long oldEpochTimeSeconds)
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::removeOldSubscriptions "
                 "oldEpochTimeSeconds = %ld",
                 oldEpochTimeSeconds);

   // Call SipSubscriptionMgr to update the in-memory data.
   SipSubscriptionMgr::removeOldSubscriptions(oldEpochTimeSeconds);

   // Update the IMDB.
   mDB.removeAllExpired();
}

// Set stored value for the next NOTIFY CSeq.
void SipPersistentSubscriptionMgr::setNextNotifyCSeq(
   const UtlString& dialogHandleString,
   int nextLocalCseq, int version)
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::setNextNotifyCSeq dialogHandleString = '%s', nextLocalCseq = %d",
                 dialogHandleString.data(), nextLocalCseq);

   SipSubscriptionMgr::setNextNotifyCSeq(dialogHandleString,
                                         nextLocalCseq, version);

   // We need to update the IMDB here, but we don't have enough information
   // to do it right.  This is because the event-type and event-id, which are
   // needed to uniquely identify a subscription.  We only have the dialog
   // handle.  OTOH, the dialog handle is what
   // SipSubscriptionMgr::setNextNotifyCSeq needs.  This really must be
   // refactored.  Fortunately, we don't use this method.
   assert(FALSE);
}

// Update the IMDB with the NOTIFY CSeq now in notifyRequest and the
// specified 'version' for the given eventTypeKey.
void SipPersistentSubscriptionMgr::updateVersion(SipMessage& notifyRequest,
                                                 int version,
                                                 const UtlString& eventTypeKey)
{
   // Call the superclass's updateVersion.
   SipSubscriptionMgr::updateVersion(notifyRequest, version, eventTypeKey);

   // Extract from the NOTIFY the information we need to find the right
   // IMDB row.
   int cseq;
   UtlString method;
   notifyRequest.getCSeqField(&cseq, &method);

   UtlString to;
   UtlString from;
   UtlString callId;
   UtlString eventHeader, eventId;
   unsigned long now;

   // Note that the "to" and "from" fields of the subscription table
   // are as those URIs appear in the SUBSCRIBE message, which is
   // reversed in the NOTIFY message.
   notifyRequest.getToField(&from);
   notifyRequest.getFromField(&to);
   notifyRequest.getCallIdField(&callId);
   notifyRequest.getEventFieldParts(&eventHeader, &eventId);
   now = OsDateTime::getSecsSinceEpoch();

   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::updateVersion "
                 "callId = '%s', to = '%s', from = '%s', eventHeader = '%s', eventTypeKey = '%s', eventId = '%s', cseq = %d, version = %d",
                 callId.data(), to.data(), from.data(), eventHeader.data(), eventTypeKey.data(), eventId.data(), cseq, version);
   mDB.updateNotifyUnexpiredSubscription(
      mComponent, to, from, callId, eventTypeKey, eventId, now, cseq, version);
}

/** get the next notify body "version" value that is allowed
 *  for a resource (as far as is known by this SipSubscriptionMgr).
 *  If no information is available, returns 0.
 */
int SipPersistentSubscriptionMgr::getNextAllowedVersion(const UtlString& resourceId)
{
   // Get any value from the SipSubscriptionMgr.cpp.
   int superclass_value =
      SipSubscriptionMgr::getNextAllowedVersion(resourceId);

   // Get the max of the <version> values whose <uri> matches resourceId.
   // Add 1, as the recorded value is the version used in the last
   // NOTIFY.
   int max = mDB.getMaxVersion(resourceId) + 1;

   return superclass_value > max ? superclass_value : max;
}
