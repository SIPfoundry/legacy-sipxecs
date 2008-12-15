// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <stdlib.h>
#include <limits.h>

// APPLICATION INCLUDES

#include "RegisterEventServer.h"
#include <os/OsSysLog.h>
#include <utl/XmlContent.h>
#include <sipdb/ResultSet.h>
#include <sipdb/RegistrationDB.h>
#include <sipdb/SIPDBManager.h>
#include <net/SipRegEvent.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


// Objects to construct default content for registration events.

class RegEventDefaultConstructor : public SipPublishContentMgrDefaultConstructor
{
  public:

   // Constructor
   RegEventDefaultConstructor(// owning RegisterEventServer
                              RegisterEventServer* mRegisterEventServer);

   // Destructor
   ~RegEventDefaultConstructor();

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

   //! The owning SipRegEventserver.
   RegisterEventServer* mpRegisterEventServer;

   static UtlContainableType TYPE;    /** < Class type used for runtime checking */
};

// Static identifier for the type.
const UtlContainableType RegEventDefaultConstructor::TYPE = "RegEventDefaultConstructor";

// Constructor
RegEventDefaultConstructor::RegEventDefaultConstructor(RegisterEventServer* parent) :
   mpRegisterEventServer(parent)
{
}

// Destructor
RegEventDefaultConstructor::~RegEventDefaultConstructor()
{
}

// Generate the default content for dialog status.
void RegEventDefaultConstructor::generateDefaultContent(SipPublishContentMgr* contentMgr,
                                                        const char* resourceId,
                                                        const char* eventTypeKey,
                                                        const char* eventType)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "RegEventDefaultConstructor::generateDefaultContent resourceId = '%s', eventTypeKey = '%s', eventType = '%s'",
                 resourceId, eventTypeKey, eventType);

   // Construct the AOR.
   Url request_uri(resourceId, TRUE);
   UtlString aor;
   request_uri.getUserId(aor);
   aor.insert(0, "sip:");
   aor.append("@");
   aor.append(*mpRegisterEventServer->getDomainName());
   Url aor_uri(aor, TRUE);

   // Construct the content.
   HttpBody* body;
   int version;
   mpRegisterEventServer->generateContent(aor, aor_uri, body, version);

   // Install it for the resource, but do not publish it, because our
   // caller will publish it.
   contentMgr->publish(resourceId, eventTypeKey, eventType, 1, &body, &version, TRUE);
}

// Make a copy of this object according to its real type.
SipPublishContentMgrDefaultConstructor* RegEventDefaultConstructor::copy()
{
   return new RegEventDefaultConstructor(mpRegisterEventServer);
}

// Get the ContainableType for a UtlContainable derived class.
UtlContainableType RegEventDefaultConstructor::getContainableType() const
{
    return RegEventDefaultConstructor::TYPE;
}


// Constructor
RegisterEventServer::RegisterEventServer(const UtlString& domainName,
                                         int tcpPort,
                                         int udpPort,
                                         int tlsPort,
                                         const UtlString& bindIp) :
   mDomainName(domainName),
   mEventType(REG_EVENT_TYPE),
   mpRegistrationDBInstance(RegistrationDB::getInstance()),
   mUserAgent(
      tcpPort, // sipTcpPort
      udpPort, // sipUdpPort
      tlsPort, // sipTlsPort
      NULL, // publicAddress
      NULL, // defaultUser
      bindIp,
      NULL, // sipProxyServers
      NULL, // sipDirectoryServers
      NULL, // sipRegistryServers
      NULL, // authenticationScheme
      NULL, // authenicateRealm
      NULL, // authenticateDb
      NULL, // authorizeUserIds
      NULL, // authorizePasswords
      NULL, // natPingUrl
      0, // natPingFrequency
      "PING", // natPingMethod
      NULL, // lineMgr
      SIP_DEFAULT_RTT, // sipFirstResendTimeout
      TRUE, // defaultToUaTransactions
      -1, // readBufferSize
      OsServerTask::DEF_MAX_MSGS, // queueSize
      FALSE // bUseNextAvailablePort
      ),
   mSubscriptionMgr(SUBSCRIPTION_COMPONENT_REG, mDomainName),
   mSubscribeServer(mUserAgent, mEventPublisher, mSubscriptionMgr,
                    mPolicyHolder)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "RegisterEventServer:: mDomainName = '%s', tcpPort = %d, udpPort = %d, tlsPort = %d",
                 mDomainName.data(), tcpPort, udpPort, tlsPort);

   // Construct addresses:
   // our local host-part
   // contact address to be used in outgoing requests (primarily SUBSCRIBEs)
   {
      UtlString localAddress;
      OsSocket::getHostIp(&localAddress);
      char buffer[100];
      sprintf(buffer, "%s:%d", localAddress.data(),
              portIsValid(udpPort) ? udpPort : tcpPort);
      mLocalHostPart = buffer;
      sprintf(buffer, "sip:sipXrls@%s:%d", localAddress.data(),
              portIsValid(udpPort) ? udpPort : tcpPort);
      mOutgoingAddress = buffer;
   }

   // Search the subscription DB to initialize the version number
   // to the largest recorded version for reg events.
   mVersion = 0;
   unsigned long now = OsDateTime::getSecsSinceEpoch();
   ResultSet rs;
   SubscriptionDB::getInstance()->getAllRows(rs);
   UtlSListIterator itor(rs);
   UtlHashMap* rowp;
   while ((rowp = dynamic_cast <UtlHashMap*> (itor())))
   {
      // First, filter for rows that have the right component and have
      // not yet expired.
      UtlString* componentp =
         dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gComponentKey));
      assert(componentp);
      unsigned long expires =
         *(dynamic_cast <UtlInt*> (rowp->findValue(&SubscriptionDB::gExpiresKey)));
      if (componentp->compareTo(SUBSCRIPTION_COMPONENT_REG) == 0 &&
          expires - now >= 0)
      {
         // Extract the values from the row.
         int version =
            *(dynamic_cast <UtlInt*> (rowp->findValue(&SubscriptionDB::gVersionKey)));
         if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
         {
            const UtlString* urip =
               dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gUriKey));
            const UtlString* contactp =
               dynamic_cast <UtlString*> (rowp->findValue(&SubscriptionDB::gContactKey));
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "RegisterEventServer:: AOR = '%s', contact = '%s', version = %d",
                          urip->data(), contactp->data(), version);
         }
         if (version >= mVersion)
         {
            mVersion = version + 1;
         }
      }
   }
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "RegisterEventServer:: mVersion = %d",
                 mVersion);

   // Initialize the SipUserAgent.
   // Set the user-agent string.
   mUserAgent.setUserAgentHeaderProperty("sipXecs/reg-event");
   mUserAgent.start();

   // Arrange to generate default content for reg events.
   mEventPublisher.publishDefault(mEventType.data(), mEventType.data(),
                                  new RegEventDefaultConstructor(this));

   // Start the SIP Subscribe Server after the initial content has
   // been published.  This ensures that early subscribers do not get
   // NOTIFYs with incomplete information.
   mSubscribeServer.enableEventType(mEventType);
   mSubscribeServer.start();
}

// Destructor
RegisterEventServer::~RegisterEventServer()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "RegisterEventServer::~ this = %p",
                 this);

   // Stop the subscribe server.
   mSubscribeServer.requestShutdown();

   // Shut down userAgent
   mUserAgent.shutdown(FALSE);
   while(!mUserAgent.isShutdownDone())
   {
      OsTask::delay(100);
   }

   // Free the Registration DB instance.
   RegistrationDB::releaseInstance();
}

// Generate and publish content for reg events for an AOR.
void RegisterEventServer::generateAndPublishContent(const UtlString& aorString,
                                                    const Url& aorUri)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "RegisterEventServer::generateAndPublishContent aorString = '%s'",
                 aorString.data());

   HttpBody* body;
   int version;
   
   generateContent(aorString, aorUri, body, version);
   mEventPublisher.publish(aorString, mEventType.data(), mEventType.data(),
                           1, &body, &version, FALSE);
}

// Generate (but not publish) content for reg events for an AOR.
void RegisterEventServer::generateContent(const UtlString& aorString,
                                          const Url& aorUri,
                                          HttpBody*& body,
                                          int& version)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "RegisterEventServer::generateContent aorString = '%s'",
                 aorString.data());

   // Use an expiraton time of 0 to get all the registrations for the
   // AOR, including the ones that have expired but not been purged.
   ResultSet rs;
   getRegistrationDBInstance()->getUnexpiredContacts(aorUri,
                                                     0,
                                                     rs);
   unsigned long now = OsDateTime::getSecsSinceEpoch();

   // Construct the body, an empty notice for the user.
   UtlString content;
   // Currently, all versions for reg event content are based on one
   // counter in the RegisterEventServer.  Since this counter persists,
   // it will count up forever.  But this crude architecture will
   // be fixed before the counter overflows for any of our users.
   // (Ugh.)
   version = mVersion++;
   content.append("<?xml version=\"1.0\"?>\r\n"
                  "<reginfo xmlns=\"urn:ietf:params:xml:ns:reginfo\" "
                  "xmlns:gr=\"urn:ietf:params:xml:ns:gruuinfo\" "
                  "version=\"");
   content.appendNumber(version);
   content.append("\" state=\"full\">\r\n");
   content.append("  <registration aor=\"");
   XmlEscape(content, aorString);
   content.append("\" id=\"");
   XmlEscape(content, aorString);
   // If there are no unexpired contacts, the state is "init", otherwise
   // "active".
   UtlSListIterator rs_itor(rs);
   UtlHashMap* rowp;
   UtlBoolean found = FALSE;
   while (!found &&
          (rowp = dynamic_cast <UtlHashMap*> (rs_itor())))
   {
      assert(rowp);
      unsigned long expired =
         (dynamic_cast <UtlInt*>
          (rowp->findValue(&RegistrationDB::gExpiresKey)))->getValue();

      if (expired >= now)
      {
         found = TRUE;
      }
   }
   content.append("\" state=\"");
   content.append(found ? "active" : "init");
   content.append("\">\r\n");

   // Iterate through the result set, generating <contact> elements
   // for each contact.
   rs_itor.reset();
   while ((rowp = dynamic_cast <UtlHashMap*> (rs_itor())))
   {
      assert(rowp);
      UtlString* callid =
         dynamic_cast <UtlString*> (rowp->findValue(&RegistrationDB::gCallidKey));
      UtlString* contact_string =
         dynamic_cast <UtlString*> (rowp->findValue(&RegistrationDB::gContactKey));
      Url contact_nameaddr(*contact_string, FALSE);
      UtlString* q =
         dynamic_cast <UtlString*> (rowp->findValue(&RegistrationDB::gQvalueKey));
      int cseq =
         (dynamic_cast <UtlInt*>
          (rowp->findValue(&RegistrationDB::gCseqKey)))->getValue();
      unsigned long expired =
         (dynamic_cast <UtlInt*>
          (rowp->findValue(&RegistrationDB::gExpiresKey)))->getValue();
      UtlString* pathVector = dynamic_cast <UtlString*> (rowp->findValue(&RegistrationDB::gPathKey));
      UtlString* gruu = dynamic_cast <UtlString*> (rowp->findValue(&RegistrationDB::gGruuKey));
      UtlString* instanceId = dynamic_cast <UtlString*> (rowp->findValue(&RegistrationDB::gInstanceIdKey));

      content.append("    <contact id=\"");
      // We key the registrations table on identity and contact URI, so
      // for the id of the <content> element, we use the concatenation of
      // AOR and contact.  We could hash these together and take 64 bits
      // if we wanted the id's to be smaller and opaque.
      XmlEscape(content, aorString);
      content.append("@@");
      XmlEscape(content, *contact_string);
      // If the contact has expired, it should be terminated/expired.
      // If it has not, it should be active/registered.
      content.append(expired < now ?
                     "\" state=\"terminated\" event=\"expired\" q=\"" :
                     "\" state=\"active\" event=\"registered\" q=\"");
      if (!(q->isNull() || q->compareTo(SPECIAL_IMDB_NULL_VALUE) == 0))
      {
         XmlEscape(content, *q);
      }
      else
      {
         content.append("1");
      }
      content.append("\" callid=\"");
      XmlEscape(content, *callid);
      content.append("\" cseq=\"");
      content.appendNumber(cseq);
      content.append("\">\r\n");

      UtlString contact_addrspec;
      contact_nameaddr.getUri(contact_addrspec);
      content.append("      <uri>");
      XmlEscape(content, contact_addrspec);
      content.append("</uri>\r\n");
      
      UtlString display_name;
      contact_nameaddr.getDisplayName(display_name);
      if (!display_name.isNull())
      {
         content.append("      <display-name>");
         XmlEscape(content, display_name);
         content.append("</display-name>\r\n");
      }
      
      // Add the path header, gruu and sip instance id info

      if(NULL != pathVector && 
         !pathVector->isNull() &&
         0 != pathVector->compareTo(SPECIAL_IMDB_NULL_VALUE))
      {
         content.append("      <unknown-param name=\"path\">");
         XmlEscape(content, *pathVector);
         content.append("</unknown-param>\r\n");
      }
      if(NULL != instanceId && 
         !instanceId->isNull() &&
         0 != instanceId->compareTo(SPECIAL_IMDB_NULL_VALUE))
      {
         content.append("      <unknown-param name=\"+sip.instance\">");
         XmlEscape(content, *instanceId);
         content.append("</unknown-param>\r\n");
      }
      if(NULL != gruu && 
         !gruu->isNull() &&
         0 != gruu->compareTo(SPECIAL_IMDB_NULL_VALUE))
      {
         content.append("      <gr:pub-gruu uri=\"");
         Url tmp(*gruu);
         tmp.setScheme(Url::SipUrlScheme);
         tmp.setGRUU( UtlString("") );
         XmlEscape(content, tmp.toString());
         content.append("\"/>\r\n");
      }

      content.append("    </contact>\r\n");
   }

   content.append("  </registration>\r\n");
   content.append("</reginfo>\r\n");

   // Build an HttpBody.
   body = new HttpBody(content, strlen(content),
                       REG_EVENT_CONTENT_TYPE);

   // Returned values are in 'body' and 'version'.
}
