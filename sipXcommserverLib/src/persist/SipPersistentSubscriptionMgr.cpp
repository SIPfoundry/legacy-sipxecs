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
#include <os/OsSysLog.h>
#include <os/OsTimer.h>
#include <os/OsDateTime.h>
#include <os/OsEventMsg.h>
#include <net/SipMessage.h>
#include <net/SipDialog.h>
#include <sipdb/ResultSet.h>
#include <sipdb/SIPDBManager.h>

// STATIC VARIABLES

// Persistence interval is 20 seconds.
const OsTime SipPersistentSubscriptionMgr::sPersistInterval(20, 0);

// A constant string containing SPECIAL_IMDB_NULL_VALUE (currently, "%"),
// which for some reason IMDB returns in place of null strings.
const UtlString special_imdb_null_value(SPECIAL_IMDB_NULL_VALUE);
// A null string to replace it with.
const UtlString null_string("");

// Constructor
SipPersistentSubscriptionMgr::SipPersistentSubscriptionMgr(
   const UtlString& component,
   const UtlString& domain,
   const UtlString& fileName) :
   mComponent(component),
   mDomain(domain),
   mSubscriptionDBInstance(SubscriptionDB::getInstance(fileName)),
   mPersistenceTimer(mPersistTask.getMessageQueue(), 0),
   mPersistTask(mSubscriptionDBInstance)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr:: "
                 "mComponent = '%s', mDomain = '%s', fileName = '%s'",
                 mComponent.data(), mDomain.data(), fileName.data());

   // Start the persist task.
   mPersistTask.start();

   // Read the subscription table and initialize the SipSubscriptionMgr.

   unsigned long now = OsDateTime::getSecsSinceEpoch();
   ResultSet rs;
   mSubscriptionDBInstance->getAllRows(rs);
   UtlSListIterator itor(rs);
   UtlHashMap* rowp;
   while ((rowp = dynamic_cast <UtlHashMap*> (itor())))
   {
      if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
      {
         UtlString out;
         UtlHashMapIterator itor(*rowp);
         UtlString* key;
         UtlContainable* value;
         while ((key = dynamic_cast <UtlString*> (itor())))
         {
            value = itor.value();
            if (!out.isNull())
            {
               out.append(", ");
            }
            out.append(*key);
            out.append(" = ");
            if (value->getContainableType() == UtlString::TYPE)
            {
               out.append("'");
               out.append(*(dynamic_cast <UtlString*> (value)));
               out.append("'");
            }
            else if (value->getContainableType() == UtlInt::TYPE)
            {
               out.appendNumber((int) (*(dynamic_cast <UtlInt*> (value))));
            }
            else
            {
               out.append(value->getContainableType());
            }
         }
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipPersistentSubscriptionMgr:: "
                       "table row: %s",
                       out.data());
      }

      // First, filter for rows that have the right component and have
      // not yet expired.
      UtlString* componentp =
         dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gComponentKey));
      assert(componentp);
      int expires =
         *(dynamic_cast <UtlInt*> (rowp->findValue(&SubscriptionDB::gExpiresKey)));
      if (componentp->compareTo(mComponent) == 0 &&
          expires - now >= 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipPersistentSubscriptionMgr:: "
                       "loading row");

         // Extract the values from the row.
         const UtlString* top =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gToKey));
         const UtlString* fromp =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gFromKey));
         const UtlString* callidp =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gCallidKey));
         const UtlString* eventtypekeyp =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gEventtypekeyKey));
         const UtlString* eventtypep =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gEventtypeKey));
         // For upward compatibility from schema subscription-00-00,
         // if the eventtypekey value is null, copy the eventtype value.
         // We can do this by just copying the pointer.  (Both pointers are into
         // the ResultSet 'rs', which owns the underlying UtlString's.)
         if (eventtypekeyp->isNull())
         {
            eventtypekeyp = eventtypep;
         }
         const UtlString* eventidp =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gIdKey));
         // Correct the null string if it is returned as
         // SPECIAL_IMDB_NULL_VALUE.
         if (eventidp->compareTo(special_imdb_null_value) == 0)
         {
            eventidp = &null_string;
         }
         int subcseq =
            *(dynamic_cast <UtlInt*> (rowp->findValue(&SubscriptionDB::gSubscribecseqKey)));
         const UtlString* urip =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gUriKey));
         const UtlString* contactp =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gContactKey));
         const UtlString* routep =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gRecordrouteKey));
         // Correct the null string if it is returned as
         // SPECIAL_IMDB_NULL_VALUE.
         if (routep->compareTo(special_imdb_null_value) == 0)
         {
            routep = &null_string;
         }
         int notifycseq =
            *(dynamic_cast <UtlInt*> (rowp->findValue(&SubscriptionDB::gNotifycseqKey)));
         const UtlString* acceptp =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gAcceptKey));
         int version =
            *(dynamic_cast <UtlInt*> (rowp->findValue(&SubscriptionDB::gVersionKey)));
         const UtlString* keyp =
            dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gKeyKey));

         // Use SipSubscriptionMgr to update the in-memory data.

         // Construct a fake SUBSCRIBE request to carry most of the data
         // that updateDialogInfo needs.
         SipMessage subscribeRequest;
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipPersistentSubscriptionMgr:: expires = %d, now = %d",
                       (int) expires, (int) now);
         subscribeRequest.setSubscribeData(urip->data(),
                                           fromp->data(),
                                           top->data(),
                                           callidp->data(),
                                           subcseq,
                                           eventtypekeyp->data(),
                                           acceptp->data(),
                                           eventidp->data(),
                                           contactp->data(),
                                           NULL,
                                           expires - now);
         // Install the saved Route as a set of Record-Route headers in the
         // SUBSCRIBE, so that insertDialogInfo will find and record the route.
         Url route_url;
         UtlString route_url_string;
         UtlString route_string(*routep);
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
         if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
         {
            UtlString m, d;
            ssize_t l;
            subscribeRequest.getBytes(&m, &l, FALSE);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipPersistentSubscriptionMgr:: subscribeRequest = '%s'",
                          m.data());
         }

         // Variables to hold the output of insertDialogInfo.
         UtlString subscribeDialogHandle;
         UtlBoolean isNew;
         UtlBoolean ret =
            SipSubscriptionMgr::insertDialogInfo(subscribeRequest,
                                                 // *keyp is the resource that
                                                 // is subscribed to.
                                                 *keyp,
                                                 *eventtypekeyp,
                                                 *eventtypep,
                                                 expires,
                                                 notifycseq,
                                                 version,
                                                 subscribeDialogHandle,
                                                 isNew);
         if (!ret)
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipPersistentSubscriptionMgr:: "
                          "SipSubscriptionMgr::insertDialogInfo failed keyp = '%s', eventtypekeyp = '%s', eventtypep = '%s', subscribeDialogHandle = '%s'",
                          keyp->data(),
                          eventtypekeyp->data(),
                          eventtypep->data(),
                          subscribeDialogHandle.data());
         }
         else
         {
            // Set the next NOTIFY CSeq value.
            // (The data in IMDB has already been set.)
            SipSubscriptionMgr::setNextNotifyCSeq(subscribeDialogHandle,
                                                  notifycseq,
                                                  version);
         }
      }
   }
}

// Destructor
SipPersistentSubscriptionMgr::~SipPersistentSubscriptionMgr()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::~");

   // Save the IMDB to disk if it is dirty.
   // Stop the timer first, so if it has fired, its message is in
   // mPersistTask's queue, and will be finished when we shut down
   // that task.
   OsStatus running = mPersistenceTimer.stop();

   // Stop the persist task.
   // This makes sure that if mPersistenceTimer fires later, it won't have
   // any effect.
   mPersistTask.requestShutdown();
   while (!mPersistTask.isShutDown())
   {
      OsTask::delay(100);
   }

   if (running == OS_SUCCESS)
   {
      // Timer was running; database was dirty.
      mSubscriptionDBInstance->store();
   }

   // Free the DB instance, if necessary.
   SubscriptionDB::releaseInstance();
}

/* ============================ MANIPULATORS ============================== */

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
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::updateDialogInfo "
                 "resourceId = '%s', eventTypeKey = '%s', eventType = '%s'",
                 resourceId.data(), eventTypeKey.data(), eventType.data());

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

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipPersistentSubscriptionMgr::updateDialogInfo "
                    "mComponent = '%s', requestUri = '%s', callId = '%s', contactEntry = '%s', expires = %d, "
                    "to = '%s', from = '%s', eventTypeKey = '%s', eventType = '%s', key = '%s', route = '%s', accept = '%s'",
                    mComponent.data(), requestUri.data(),
                    callId.data(), contactEntry.data(), expires,
                    to.data(), from.data(), eventTypeKey.data(),
                    eventType.data(), resourceId.data(), route.data(),
                    accept.data());

      // Attempt to update an existing row.
      int now = (int)OsDateTime::getSecsSinceEpoch();
      ret = mSubscriptionDBInstance->updateSubscribeUnexpiredSubscription(
         mComponent, to, from, callId, eventTypeKey, "",
         now, expires, subscribeCseq);

      if (!ret)
      {
         // Add a new row.

         // This call assumes that eventTypeKey (as set by the
         // handler) is OK for use as the <eventtypekey>,
         // and that the NOTIFY CSeq's will start at 1.  0 is used as
         // the initial XML version.
         ret = mSubscriptionDBInstance->insertRow(
            mComponent, requestUri, callId, contactEntry,
            expires, subscribeCseq, eventTypeKey, eventType, "",
            to, from, resourceId, route, 1, accept, 0);

         if (!ret)
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipPersistantSubscriptionMgr::addSubscription "
                          "Could not update or insert record in database");
         }
      }

      // Start the save timer.
      mPersistenceTimer.oneshotAfter(sPersistInterval);
   }

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
   UtlString* acceptHeaderValue)
{
   UtlBoolean ret;

   // Call SipSubscriptionMgr to update the in-memory data.
   ret = SipSubscriptionMgr::getNotifyDialogInfo(subscribeDialogHandle,
                                                 notifyRequest,
                                                 subscriptionStateFormat,
                                                 resourceId,
                                                 eventTypeKey,
                                                 eventType,
                                                 acceptHeaderValue);

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
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
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
      mSubscriptionDBInstance->updateToTag(callId, localTag, remoteTag);
   }

   // The NOTIFY CSeq and XML version will be saved when our caller
   // calls updateVersion().

   return ret;
}

UtlBoolean SipPersistentSubscriptionMgr::endSubscription(const UtlString& dialogHandle,
                                                         enum SipSubscriptionMgr::subscriptionChange change)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
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
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipPersistentSubscriptionMgr::endSubscription callId = '%s', localTag = '%s', remoteTag = '%s'",
                    callId.data(), localTag.data(), remoteTag.data());

      if (mSubscriptionDBInstance->findFromAndTo(callId, localTag, remoteTag,
                                                 from, to))
      {
         // We use the largest possible CSeq number here, as we assume
         // that no more than one subscription is established in a given dialog.
         // That assumption is incorrect, but the rest of the SipSubscriptionMgr
         // machinery makes that assumption, and does not provide us with the CSeq
         // value, nor with the event type and id, which are necessary to uniquely
         // identify a subscription.  (See RFC 3265, section 7.2.1.)
         mSubscriptionDBInstance->removeRow(mComponent, to, from, callId,
                                            0x7FFFFFFF);

         // Start the save timer.
         mPersistenceTimer.oneshotAfter(sPersistInterval);
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipPersistentSubscriptionMgr::endSubscription "
                       "Cannot find subscription for dialog handle '%s'",
                       dialogHandle.data());
      }
   }

   return ret;
}

void SipPersistentSubscriptionMgr::removeOldSubscriptions(long oldEpochTimeSeconds)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::removeOldSubscriptions "
                 "oldEpochTimeSeconds = %ld",
                 oldEpochTimeSeconds);

   // Call SipSubscriptionMgr to update the in-memory data.
   SipSubscriptionMgr::removeOldSubscriptions(oldEpochTimeSeconds);

   // Update the IMDB.
   mSubscriptionDBInstance->removeExpired(mComponent, oldEpochTimeSeconds);

   // Start the save timer.
   mPersistenceTimer.oneshotAfter(sPersistInterval);
}

// Set stored value for the next NOTIFY CSeq and version.
void SipPersistentSubscriptionMgr::setNextNotifyCSeq(
   const UtlString& dialogHandleString,
   int nextLocalCseq,
   int version)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::setNextNotifyCSeq dialogHandleString = '%s', nextLocalCseq = %d, version = %d",
                 dialogHandleString.data(), nextLocalCseq, version);

   SipSubscriptionMgr::setNextNotifyCSeq(dialogHandleString,
                                         nextLocalCseq,
                                         version);

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
   int now;

   // Note that the "to" and "from" fields of the subscription table
   // are as those URIs appear in the SUBSCRIBE message, which is
   // reversed in the NOTIFY message.
   notifyRequest.getToField(&from);
   notifyRequest.getFromField(&to);
   notifyRequest.getCallIdField(&callId);
   notifyRequest.getEventField(&eventHeader, &eventId);
   now = (int) OsDateTime::getSecsSinceEpoch();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgr::updateVersion "
                 "callId = '%s', to = '%s', from = '%s', eventHeader = '%s', eventTypeKey = '%s', eventId = '%s', cseq = %d, version = %d",
                 callId.data(), to.data(), from.data(), eventHeader.data(), eventTypeKey.data(), eventId.data(), cseq, version);
   mSubscriptionDBInstance->updateNotifyUnexpiredSubscription(
      mComponent, to, from, callId, eventTypeKey, eventId, now, cseq, version);

   // Start the save timer.
   mPersistenceTimer.oneshotAfter(sPersistInterval);
}

/* ============================ ACCESSORS ================================= */

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
   int max = mSubscriptionDBInstance->getMaxVersion(resourceId) + 1;

   return superclass_value > max ? superclass_value : max;
}
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


// Constructor
SipPersistentSubscriptionMgrTask::SipPersistentSubscriptionMgrTask(
   SubscriptionDB* subscriptionDBInstance) :
   OsServerTask("SipPersistentSubscriptionMgrTask-%d"),
   mSubscriptionDBInstance(subscriptionDBInstance)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgrTask:: "
                 "mSubscriptionDBInstance = %p",
                 mSubscriptionDBInstance);
}

// Destructor
SipPersistentSubscriptionMgrTask::~SipPersistentSubscriptionMgrTask()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPersistentSubscriptionMgrTask::~");
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean SipPersistentSubscriptionMgrTask::handleMessage(OsMsg& rMsg)
{
   UtlBoolean handled = FALSE;

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "SipPersistentSubscriptionMgrTask::handleMessage message type %d subtype %d",
                 rMsg.getMsgType(), rMsg.getMsgSubType());

   if (rMsg.getMsgType() == OsMsg::OS_EVENT &&
       rMsg.getMsgSubType() == OsEventMsg::NOTIFY)
   {
      // An event notification.
      // The only event is an order to store the database to disk.
      mSubscriptionDBInstance->store();
      handled = TRUE;
   }
   else if (rMsg.getMsgType() == OsMsg::OS_SHUTDOWN)
   {
      // Leave 'handled' false and pass on to OsServerTask::handleMessage.
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_ERR,
                    "SipPersistentSubscriptionMgrTask::handleMessage unknown msg type %d",
                    rMsg.getMsgType());
   }

   return handled;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
