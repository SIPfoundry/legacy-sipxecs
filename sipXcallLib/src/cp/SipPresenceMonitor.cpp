//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsEventMsg.h>
#include <os/OsFS.h>
#include <os/OsMsg.h>
#include <os/OsSysLog.h>
#include <os/OsConfigDb.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/XmlContent.h>
#include <net/NetMd5Codec.h>
#include <net/SipMessage.h>
#include <net/SipResourceList.h>
#include <cp/SipPresenceMonitor.h>
#include <cp/XmlRpcSignIn.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>
#include "xmlparser/tinyxml.h"
#include <xmlparser/ExtractContent.h>

#ifndef EXCLUDE_STREAMING
#include <mp/MpMediaTask.h>
#include <mp/NetInTask.h>
#endif

#ifdef INCLUDE_RTCP
#include <rtcp/RTCManager.h>
#endif // INCLUDE_RTCP

// DEFINES
#define RTP_START_PORT          12000    // Starting RTP port
#define MAX_CONNECTIONS         200     // Max number of sim. conns

#define CONFIG_SETTING_HTTP_PORT              "SIP_PRESENCE_HTTP_PORT"
#define PRESENCE_DEFAULT_HTTP_PORT            8111

// The presence status we attribute to resources that we have no
// information about.
#define DEFAULT_PRESENCE_STATUS STATUS_CLOSED

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// STATIC VARIABLE INITIALIZATIONS

// Persistence interval is 20 seconds.
const OsTime SipPresenceMonitor::sPersistInterval(20, 0);

// The 'type' attribute of the top-level 'items' element.
const UtlString SipPresenceMonitor::sType("presence-state");

// The XML namespace of the top-level 'items' element.
const UtlString SipPresenceMonitor::sXmlNamespace("http://www.sipfoundry.org/sipX/schema/xml/presence-state-00-00");

// Objects to construct default content for presence events.

class PresenceDefaultConstructor : public SipPublishContentMgrDefaultConstructor
{
  public:

   /** Generate the content for a resource and event.
    */
   virtual void generateDefaultContent(SipPublishContentMgr* contentMgr,
                                       const char* resourceId,
                                       const char* eventTypeKey,
                                       const char* eventType);

   /// Make a copy of this object according to its real type.
   virtual SipPublishContentMgrDefaultConstructor* copy();

   // Service routine for UtlContainable.
   virtual const char* const getContainableType() const;

protected:
   static UtlContainableType TYPE;    /** < Class type used for runtime checking */
};

// Static identifier for the type.
const UtlContainableType PresenceDefaultConstructor::TYPE = "PresenceDefaultConstructor";

// Generate the default content for presence status.
void PresenceDefaultConstructor::generateDefaultContent(SipPublishContentMgr* contentMgr,
							const char* resourceId,
							const char* eventTypeKey,
							const char* eventType)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "PresenceDefaultConstructor::generateDefaultContent "
                 "generating default content for resourceId '%s', "
                 "eventTypeKey '%s', eventType '%s'",
                 resourceId, eventTypeKey, eventType);

   // Create a presence event package and store it in the publisher.
   // This code parallels SipPresenceMonitor::setStatus.
   SipPresenceEvent* sipPresenceEvent = new SipPresenceEvent(resourceId);

   UtlString id;
   UtlString resource(resourceId);
   SipPresenceMonitor::makeId(id, resource);
   Tuple* tuple = new Tuple(id.data());
   tuple->setStatus(DEFAULT_PRESENCE_STATUS);
   tuple->setContact(resourceId, 1.0);

   sipPresenceEvent->insertTuple(tuple);

   // Build its text version.
   sipPresenceEvent->buildBody();

   // Publish the event (storing it for the resource), but set
   // noNotify to TRUE, because our caller will push the NOTIFYs.
   contentMgr->publish(resourceId, eventTypeKey, eventType, 1,
                       &(HttpBody*&) sipPresenceEvent, TRUE);
}

// Make a copy of this object according to its real type.
SipPublishContentMgrDefaultConstructor* PresenceDefaultConstructor::copy()
{
   // Copying these objects is easy, since they have no member variables, etc.
   return new PresenceDefaultConstructor;
}

// Get the ContainableType for a UtlContainable derived class.
UtlContainableType PresenceDefaultConstructor::getContainableType() const
{
    return PresenceDefaultConstructor::TYPE;
}


// Constructor
SipPresenceMonitor::SipPresenceMonitor(SipUserAgent* userAgent,
                                       SipSubscriptionMgr* subscriptionMgr,
                                       UtlString& domainName,
                                       int hostPort,
                                       OsConfigDb* configFile,
                                       bool toBePublished,
                                       const char* persistentFile) :
   mpUserAgent(userAgent),
   mDomainName(domainName),
   mToBePublished(toBePublished),
   mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mpSubscriptionMgr(subscriptionMgr),
   mPersistentFile(persistentFile),
   mPersistenceTimer(mPersistTask.getMessageQueue(), 0),
   mPersistTask(this)
{
   // Read the persistent file to initialize mPersenceEventList, if
   // one is supplied.
   if (!mPersistentFile.isNull())
   {
      readPersistentFile();
   }

   char buffer[80];
   sprintf(buffer, "@%s:%d", mDomainName.data(), hostPort);
   mHostAndPort = UtlString(buffer);

   UtlString localAddress;
   OsSocket::getHostIp(&localAddress);

   OsConfigDb configDb;
   configDb.set("PHONESET_MAX_ACTIVE_CALLS_ALLOWED", 2*MAX_CONNECTIONS);

#ifdef INCLUDE_RTCP
   CRTCManager::getRTCPControl();
#endif //INCLUDE_RTCP
   // Start the media processing tasks.
   mpStartTasks();

   // Instantiate the call processing subsystem
   mpCallManager = new CallManager(FALSE,
                                   NULL,
                                   TRUE,                              // early media in 180 ringing
                                   &mCodecFactory,
                                   RTP_START_PORT,                    // rtp start
                                   RTP_START_PORT + (2*MAX_CONNECTIONS), // rtp end
                                   localAddress,
                                   localAddress,
                                   mpUserAgent,
                                   0,                                 // sipSessionReinviteTimer
                                   NULL,                              // mgcpStackTask
                                   NULL,                              // defaultCallExtension
                                   Connection::RING,                  // availableBehavior
                                   NULL,                              // unconditionalForwardUrl
                                   -1,                                // forwardOnNoAnswerSeconds
                                   NULL,                              // forwardOnNoAnswerUrl
                                   Connection::BUSY,                  // busyBehavior
                                   NULL,                              // sipForwardOnBusyUrl
                                   NULL,                              // speedNums
                                   CallManager::SIP_CALL,             // phonesetOutgoingCallProtocol
                                   4,                                 // numDialPlanDigits
                                   CallManager::NEAR_END_HOLD,        // holdType
                                   5000,                              // offeringDelay
                                   "",                                // pLocal
                                   CP_MAXIMUM_RINGING_EXPIRE_SECONDS, // inviteExpiresSeconds
                                   QOS_LAYER3_LOW_DELAY_IP_TOS,       // expeditedIpTos
                                   MAX_CONNECTIONS,                   // maxCalls
                                   sipXmediaFactoryFactory(&configDb));    // CpMediaInterfaceFactory

   mpDialInServer = new PresenceDialInServer(mpCallManager, configFile);
   mpCallManager->addTaoListener(mpDialInServer);
   mpDialInServer->start();

   // Start the call processing system
   mpCallManager->start();

   // Add self to the presence dial-in server for state change notification
   mpDialInServer->addStateChangeNotifier("Presence_Dial_In_Server", this);

   if (mToBePublished)
   {
      // Create the SIP Subscribe Server
      mpSubscribeServer = new SipSubscribeServer(*mpUserAgent, mSipPublishContentMgr,
                                                 *mpSubscriptionMgr, mPolicyHolder);
      // Arrange to generate default content for presence events.
      mSipPublishContentMgr.publishDefault(PRESENCE_EVENT_TYPE, PRESENCE_EVENT_TYPE,
                                           new PresenceDefaultConstructor);
      mpSubscribeServer->enableEventType(PRESENCE_EVENT_TYPE);
      mpSubscribeServer->start();
   }

   // Enable the xmlrpc sign-in/sign-out
   int HttpPort;
   if (configDb.get(CONFIG_SETTING_HTTP_PORT, HttpPort) != OS_SUCCESS)
   {
      HttpPort = PRESENCE_DEFAULT_HTTP_PORT;
   }

   mpXmlRpcSignIn = new XmlRpcSignIn(this, HttpPort);

   // Start the persist task.
   if (!mPersistentFile.isNull())
   {
      mPersistTask.start();
   }
}

// Destructor
SipPresenceMonitor::~SipPresenceMonitor()
{
   // Remove self from the presence dial-in server
   mpDialInServer->removeStateChangeNotifier("Presence_Dial_In_Server");

   if (mpSubscribeServer)
   {
      delete mpSubscribeServer;
   }

   mMonitoredLists.destroyAll();

   mStateChangeNotifiers.destroyAll();

   if (mpXmlRpcSignIn)
   {
      delete mpXmlRpcSignIn;
   }

   // Stop the persist task and then write the event list to disk.
   mPersistenceTimer.stop();
   mPersistTask.stop();
   if (!mPersistentFile.isNull())
   {
      writePersistentFile();
   }

   mPresenceEventList.destroyAll();
}

bool SipPresenceMonitor::addExtension(UtlString& groupName, Url& contactUrl)
{
   bool result = false;
   mLock.acquire();

   // Check whether the group has already existed. If not, create one.
   SipResourceList* list = dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(&groupName));
   if (list == NULL)
   {
      UtlString* listName = new UtlString(groupName);
      list = new SipResourceList((UtlBoolean)TRUE, listName->data(), PRESENCE_EVENT_TYPE);

      mMonitoredLists.insertKeyAndValue(listName, list);
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceMonitor::addExtension insert listName %s and object %p to the resource list",
                    groupName.data(), list);
   }

   // Check whether the contact has already been added to the group
   UtlString resourceId;
   contactUrl.getIdentity(resourceId);
   Resource* resource = list->getResource(resourceId);
   if (resource == NULL)
   {
      resource = new Resource(resourceId);

      UtlString userName;
      contactUrl.getDisplayName(userName);
      resource->setName(userName);

      UtlString id;
      makeId(id, resourceId);
      resource->setInstance(id, STATE_PENDING);
      list->insertResource(resource);

      result = true;
   }
   else
   {
      OsSysLog::add(FAC_LOG, PRI_WARNING,
                    "SipPresenceMonitor::addExtension contact %s already exists.",
                    resourceId.data());
   }

   int dummy;
   list->buildBody(&dummy);

   mLock.release();
   return result;
}

bool SipPresenceMonitor::removeExtension(UtlString& groupName, Url& contactUrl)
{
   bool result = false;
   mLock.acquire();
   // Check whether the group exists or not. If not, return false.
   SipResourceList* list = dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(&groupName));
   if (list == NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceMonitor::removeExtension group %s does not exist",
                    groupName.data());
   }
   else
   {
      // Check whether the contact exists or not
      UtlString resourceId;
      contactUrl.getIdentity(resourceId);
      Resource* resource = list->getResource(resourceId);
      if (resource)
      {
         resource = list->removeResource(resource);
         delete resource;

         result = true;
      }
      else
      {
         OsSysLog::add(FAC_LOG, PRI_WARNING,
                       "SipPresenceMonitor::removeExtension subscription for contact %s does not exists.",
                       resourceId.data());
      }
   }

   mLock.release();
   return result;
}

// Returns TRUE if the requested state is different from the current state.
bool SipPresenceMonitor::addPresenceEvent(UtlString& contact, SipPresenceEvent* presenceEvent)
{
   mLock.acquire();

   bool requiredPublish = false;

   if (mPresenceEventList.find(&contact) == NULL)
   {
      requiredPublish = true;
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipPresenceMonitor::addPresenceEvent adding presenceEvent %p for contact %s",
                    presenceEvent, contact.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipPresenceMonitor::addPresenceEvent presenceEvent %p for contact %s already exists, updating its contents.",
                    presenceEvent, contact.data());

      // Get the object from the presence event list
      UtlContainable* oldKey;
      UtlContainable* foundValue;
      foundValue = mPresenceEventList.findValue(&contact);
      SipPresenceEvent* oldPresenceEvent = dynamic_cast <SipPresenceEvent *> (foundValue);
      UtlString oldStatus, status;
      UtlString id;
      makeId(id, contact);
      oldPresenceEvent->getTuple(id)->getStatus(oldStatus);
      presenceEvent->getTuple(id)->getStatus(status);

      if (status.compareTo(oldStatus) != 0)
      {
         requiredPublish = true;

         // Since we will be saving a new value, remove the old one.
         oldKey = mPresenceEventList.removeKeyAndValue(&contact, foundValue);
         delete oldKey;
         if (oldPresenceEvent)
         {
            delete oldPresenceEvent;
         }
      }
   }

   if (requiredPublish)
   {
      // Insert it into the presence event list.
      presenceEvent->buildBody();
      mPresenceEventList.insertKeyAndValue(new UtlString(contact), presenceEvent);
      if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
      {
         UtlString b;
         ssize_t l;
         presenceEvent->getBytes(&b, &l);

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipPresenceMonitor::addPresenceEvent presenceEvent %p for contact '%s' body '%s' is different from previous presenceEvent",
                       presenceEvent, contact.data(), b.data());
      }

      if (mToBePublished)
      {
         // Publish the content to the resource list.
         publishContent(contact, presenceEvent);
      }

      // Notify the state change.
      notifyStateChange(contact, presenceEvent);

      if (!mPersistentFile.isNull())
      {
         // Start the save timer.
         mPersistenceTimer.oneshotAfter(sPersistInterval);
      }
   }
   else
   {
      // Since this presenceEvent will not be published (it does not
      // change the state we've sent out), delete it now.
      delete presenceEvent;
   }

   mLock.release();
   return requiredPublish;
}

// Construct a tuple id from a presence resource name.
void SipPresenceMonitor::makeId(UtlString& id,             ///< output: tuple id
                                const UtlString& resource  ///< resource URI
   )
{
   // Construct the id by hashing the resource URI.  But we must prepend
   // a letter, because tuple id's can't start with digits.
   // (Tuple id's are defined in RFC 3863 section 4.4 to be type 'ID' from
   // http://www.w3.org/2001/XMLSchema.
   // http://www.w3.org/TR/xmlschema-2/#ID tells that their grammar is
   // "NCName" from Namespaces in XML.
   // http://www.w3.org/TR/1999/REC-xml-names-19990114/#NT-NCName gives the
   // grammar.)
   NetMd5Codec::encode(resource.data(), id); // clears previous contents of 'id'.
   id.insert(0, 'I');
}

// Read the presence events from the persistent file.
void SipPresenceMonitor::readPersistentFile()
{
   mLock.acquire();

   // Initialize Tiny XML document object.
   TiXmlDocument document;
   TiXmlNode* items_node;
   if (
      // Load the XML into it.
      document.LoadFile(mPersistentFile.data()) &&
      // Find the top element, which should be an <items>.
      (items_node = document.FirstChild("items")) != NULL &&
      items_node->Type() == TiXmlNode::ELEMENT)
   {
      // Find all the <item> elements.
      int item_seq_no = 0;
      for (TiXmlNode* item_node = 0;
           (item_node = items_node->IterateChildren("item",
                                                    item_node));
         )
      {
         if (item_node->Type() == TiXmlNode::ELEMENT)
         {
            item_seq_no++;
            TiXmlElement* item_element = item_node->ToElement();

            // Process the <item> element.
            bool item_valid = true;

            // Process the 'id' child.
            UtlString id;
            TiXmlNode* id_node = item_element->FirstChild("id");
            if (id_node && id_node->Type() == TiXmlNode::ELEMENT)
            {
               textContentShallow(id, id_node);
               if (id.isNull())
               {
                  // Id null.
                  OsSysLog::add(FAC_ACD, PRI_ERR,
                                "id child of <item> was null");
                  item_valid = false;
               }
            }
            else
            {
               // Id missing.
               OsSysLog::add(FAC_ACD, PRI_ERR,
                             "id child of <item> was missing");
               item_valid = false;
            }

            // Process the 'contact' child.
            UtlString contact;
            TiXmlNode* contact_node = item_element->FirstChild("contact");
            if (contact_node && contact_node->Type() == TiXmlNode::ELEMENT)
            {
               textContentShallow(contact, contact_node);
               if (contact.isNull())
               {
                  // Contact null.
                  OsSysLog::add(FAC_ACD, PRI_ERR,
                                "contact child of <item> was null");
                  item_valid = false;
               }
            }
            else
            {
               // Contact missing.
               OsSysLog::add(FAC_ACD, PRI_ERR,
                             "contact child of <item> was missing");
               item_valid = false;
            }

            // Process the 'status' child.
            UtlString status;
            TiXmlNode* status_node = item_element->FirstChild("status");
            if (status_node && status_node->Type() == TiXmlNode::ELEMENT)
            {
               textContentShallow(status, status_node);
               if (status.isNull())
               {
                  // Status null.
                  OsSysLog::add(FAC_ACD, PRI_ERR,
                                "status child of <item> was null");
                  item_valid = false;
               }
            }
            else
            {
               // Status missing.
               OsSysLog::add(FAC_ACD, PRI_ERR,
                             "status child of <item> was missing");
               item_valid = false;
            }

            OsSysLog::add(FAC_ACD, PRI_DEBUG,
                          "SipPresenceMonitor::readPersistentFile row: id = '%s', contact = '%s', status = '%s'",
                          id.data(), contact.data(), status.data());

            if (item_valid)
            {
               // Create a presence event package and store it in
               // mPresenceEventList.
               SipPresenceEvent* sipPresenceEvent = new SipPresenceEvent(contact);
               Tuple* tuple = new Tuple(id.data());
               tuple->setStatus(status);
               tuple->setContact(contact, 1.0);
               sipPresenceEvent->insertTuple(tuple);
               sipPresenceEvent->buildBody();
               mPresenceEventList.insertKeyAndValue(new UtlString(contact),
                                                    sipPresenceEvent);
            }
            else
            {
            OsSysLog::add(FAC_ACD, PRI_ERR,
                          "In presence status file '%s', <item> number %d had invalid or incomplete information.  The readable information was: id = '%s', contact = '%s', status = '%s'",
                          mPersistentFile.data(), item_seq_no,
                          id.data(), contact.data(), status.data());

            }
         }
      }
   }
   else
   {
      // Report error parsing file.
      OsSysLog::add(FAC_ACD, PRI_CRIT,
                    "Presence status file '%s' could not be parsed.",
                    mPersistentFile.data());
   }

   mLock.release();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPresenceMonitorPersistenceTask::readPersistentFile done");
}

// Write the presence events to the persistent file.
void SipPresenceMonitor::writePersistentFile()
{
   mLock.acquire();

   // Create an empty document
   TiXmlDocument document;

   // Create a hard coded standalone declaration section
   document.Parse("<?xml version=\"1.0\" standalone=\"yes\"?>");

   // Create the root node container
   TiXmlElement itemsElement ("items");
   itemsElement.SetAttribute("type", sType.data());
   itemsElement.SetAttribute("xmlns", sXmlNamespace.data());

   int timeNow = (int)OsDateTime::getSecsSinceEpoch();
   itemsElement.SetAttribute("timestamp", timeNow);

   // mPresenceEventList is a hash map that maps contacts to SipPresenceEvents.
   // SipPresenceEvents are hash maps of contacts to Tuples.  In practice,
   // a SipPresenceEvent has only one Tuple.  (And if it had more, there
   // would be no way to discover what they are, as there is no iterator.)
   // Tuples are triples:  id, contact, status.

   // Loop through all the events in mPresenceEventList.
   UtlHashMapIterator iterator(mPresenceEventList);
   UtlString* contact;
   while ((contact = dynamic_cast <UtlString*> (iterator())))
   {
      // Create an item container
      TiXmlElement itemElement ("item");

      // Get the SipPresenceEvent.
      SipPresenceEvent* event =
         dynamic_cast <SipPresenceEvent*> (iterator.value());

      // Calculate the id of the Tuple.
      UtlString id;
      makeId(id, *contact);

      // Get the Tuple.
      Tuple* tuple = event->getTuple(id);

      // Get the status.
      UtlString status;
      tuple->getStatus(status);

      // Construct the <item>.
      TiXmlElement id_element("id");
      TiXmlText id_content(id);
      id_element.InsertEndChild(id_content);
      itemElement.InsertEndChild(id_element);
      TiXmlElement contact_element("contact");
      TiXmlText contact_content(contact->data());
      contact_element.InsertEndChild(contact_content);
      itemElement.InsertEndChild(contact_element);
      TiXmlElement status_element("status");
      TiXmlText status_content(status);
      status_element.InsertEndChild(status_content);
      itemElement.InsertEndChild(status_element);

      // Add the <item> element to the <items> element.
      itemsElement.InsertEndChild ( itemElement );
   }

   // Attach <items> to the root node to the document
   document.InsertEndChild(itemsElement);
   document.SaveFile(mPersistentFile);

   mLock.release();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPresenceMonitorPersistenceTask::writePersistentFile file written");
}

void SipPresenceMonitor::publishContent(UtlString& contact, SipPresenceEvent* presenceEvent)
{
#ifdef SUPPORT_RESOURCE_LIST
   // Loop through all the resource lists
   UtlHashMapIterator iterator(mMonitoredLists);
   UtlString* listUri;
   SipResourceList* list;
   Resource* resource;
   UtlString id, state;
   while (listUri = dynamic_cast <UtlString *> (iterator()))
   {
      bool contentChanged = false;

      list = dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(listUri));
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceMonitor::publishContent listUri %s list %p",
                    listUri->data(), list);

      // Search for the contact in this list
      resource = list->getResource(contact);
      if (resource)
      {
         resource->getInstance(id, state);

         if (presenceEvent->isEmpty())
         {
            resource->setInstance(id, STATE_TERMINATED);
         }
         else
         {
            UtlString id;
            makeId(id, contact);
            Tuple* tuple = presenceEvent->getTuple(id);

            UtlString status;
            tuple->getStatus(status);

            if (status.compareTo(STATUS_CLOSED) == 0)
            {
               resource->setInstance(id, STATE_TERMINATED);
            }
            else
            {
               resource->setInstance(id, STATE_ACTIVE);
            }
         }

         list->buildBody();
         contentChanged = true;
      }

      if (contentChanged)
      {
         // Publish the content to the subscribe server
         // Make a copy, because mpSipPublishContentMgr will own it.
         HttpBody* pHttpBody = new HttpBody(*(HttpBody*)list);
         mSipPublishContentMgr.publish(listUri->data(),
                                       PRESENCE_EVENT_TYPE, PRESENCE_EVENT_TYPE,
                                       1, &pHttpBody);
      }
   }
#endif

   // Publish the content to the subscribe server
   // Make a copy, because mpSipPublishContentMgr will own it.
   HttpBody* pHttpBody = new HttpBody(*(HttpBody*)presenceEvent);
   mSipPublishContentMgr.publish(contact.data(),
                                 PRESENCE_EVENT_TYPE, PRESENCE_EVENT_TYPE,
                                 1, &pHttpBody);
}


void SipPresenceMonitor::addStateChangeNotifier(const char* fileUrl, StateChangeNotifier* notifier)
{
   mLock.acquire();
   UtlString* name = new UtlString(fileUrl);
   UtlVoidPtr* value = new UtlVoidPtr(notifier);
   mStateChangeNotifiers.insertKeyAndValue(name, value);
   mLock.release();
}

void SipPresenceMonitor::removeStateChangeNotifier(const char* fileUrl)
{
   mLock.acquire();
   UtlString name(fileUrl);
   mStateChangeNotifiers.destroy(&name);
   mLock.release();
}

// Caller must hold mLock.
void SipPresenceMonitor::notifyStateChange(UtlString& contact,
                                           SipPresenceEvent* presenceEvent)
{
   // Loop through the notifier list
   UtlHashMapIterator iterator(mStateChangeNotifiers);
   UtlString* listUri;
   StateChangeNotifier* notifier;
   Url contactUrl(contact);

   while ((listUri = dynamic_cast <UtlString *> (iterator())))
   {
      notifier = dynamic_cast <StateChangeNotifier *> (mStateChangeNotifiers.findValue(listUri));

      UtlString id;
      makeId(id, contact);
      Tuple* tuple = presenceEvent->getTuple(id);

      if (tuple)
      {
         UtlString status;
         tuple->getStatus(status);

         notifier->setStatus(contactUrl,
                             status.compareTo(STATUS_CLOSED) == 0 ?
                             StateChangeNotifier::AWAY :
                             StateChangeNotifier::PRESENT);
      }
      else
      {
         notifier->setStatus(contactUrl, StateChangeNotifier::AWAY);
      }
   }
}

// Returns TRUE if the requested state is different from the current state.
bool SipPresenceMonitor::setStatus(const Url& aor, const Status value)
{
   if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
   {
      UtlString aorString;
      aor.toString(aorString);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipPresenceMonitor::setStatus aor = '%s', value = %d %s",
                    aorString.data(), value,
                    (value == StateChangeNotifier::PRESENT ? "PRESENT" :
                     value == StateChangeNotifier::AWAY ? "AWAY" :
                     "UNKNOWN"));
   }

   bool result = false;

   UtlString contact;
   aor.getUserId(contact);
   contact += mHostAndPort;
   // Make the contact be a proper URI by prepending "sip:".
   contact.prepend("sip:");

   // Create a presence event package and store it in the publisher
   SipPresenceEvent* sipPresenceEvent = new SipPresenceEvent(contact);

   UtlString id;
   makeId(id, contact);

   Tuple* tuple = new Tuple(id.data());

   tuple->setStatus(value == StateChangeNotifier::PRESENT ?
                    STATUS_OPEN :
                    STATUS_CLOSED);
   tuple->setContact(contact, 1.0);

   sipPresenceEvent->insertTuple(tuple);

   // Add the SipPresenceEvent object to the presence event list.
   result = addPresenceEvent(contact, sipPresenceEvent);

   return result;
}


void SipPresenceMonitor::getState(const Url& aor, UtlString& status)
{
   UtlString contact;
   aor.getUserId(contact);
   contact += mHostAndPort;
   // Make the contact be a proper URI by prepending "sip:".
   contact.prepend("sip:");

   UtlContainable* foundValue;

   mLock.acquire();

   foundValue = mPresenceEventList.findValue(&contact);

   if (foundValue)
   {
      SipPresenceEvent* presenceEvent = dynamic_cast <SipPresenceEvent *> (foundValue);
      UtlString id;
      makeId(id, contact);
      presenceEvent->getTuple(id)->getStatus(status);
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipPresenceMonitor::getState contact %s state = %s",
                    contact.data(), status.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipPresenceMonitor::getState contact %s does not exist",
                    contact.data());

      status = STATUS_CLOSED;
   }

   mLock.release();
}


// Constructor
SipPresenceMonitorPersistenceTask::SipPresenceMonitorPersistenceTask(
   SipPresenceMonitor* presenceMonitor) :
   mSipPresenceMonitor(presenceMonitor)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPresenceMonitorPersistenceTask::_ "
                 "mSipPresenceMonitor = %p",
                 mSipPresenceMonitor);
}

// Destructor
SipPresenceMonitorPersistenceTask::~SipPresenceMonitorPersistenceTask()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPresenceMonitorPersistenceTask::~");
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean SipPresenceMonitorPersistenceTask::handleMessage(OsMsg& rMsg)
{
   UtlBoolean handled = FALSE;

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "SipPresenceMonitorPersistenceTask::handleMessage message type %d subtype %d",
                 rMsg.getMsgType(), rMsg.getMsgSubType());

   if (rMsg.getMsgType() == OsMsg::OS_EVENT &&
       rMsg.getMsgSubType() == OsEventMsg::NOTIFY)
   {
      // An event notification.
      // The only event is an order to store the database to disk.
      mSipPresenceMonitor->writePersistentFile();
      handled = TRUE;
   }
   else if (rMsg.getMsgType() == OsMsg::OS_SHUTDOWN)
   {
      // Leave 'handled' false and pass on to OsServerTask::handleMessage.
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_ERR,
                    "SipPresenceMonitorPersistenceTask::handleMessage unknown msg type %d",
                    rMsg.getMsgType());
   }

   return handled;
}

// Stop the task properly.
void SipPresenceMonitorPersistenceTask::stop()
{
   waitUntilShutDown();
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
