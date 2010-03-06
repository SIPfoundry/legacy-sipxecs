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

#define URI_IN_PREFIX "~~in~"

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

// Generate the default content for registration status.
void RegEventDefaultConstructor::generateDefaultContent(SipPublishContentMgr* contentMgr,
                                                        const char* resourceId,
                                                        const char* eventTypeKey,
                                                        const char* eventType)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "RegEventDefaultConstructor::generateDefaultContent resourceId = '%s', eventTypeKey = '%s', eventType = '%s'",
                 resourceId, eventTypeKey, eventType);

   // Outputs of the following processing.
   HttpBody* body;

   // Extract the user-part.
   Url request_uri(resourceId, TRUE);
   UtlString user;
   request_uri.getUserId(user);

   if (user.index(URI_IN_PREFIX) == 0)
   {
      // This is a ~~in~ URI.
      // Check for an '&' separator.
      ssize_t s = user.last('&');
      if (s != UTL_NOT_FOUND)
      {
         // This is a ~~in~[user]&[instrument] URI.
         const char* instrumentp = user.data() + s + 1;
         UtlString u;
         u.append(user,
                  sizeof (URI_IN_PREFIX) - 1,
                  s - (sizeof (URI_IN_PREFIX) - 1));

         // Construct the registration AOR.
         UtlString aor;
         Url aor_uri;
         aor.append("sip:");
         aor.append(u);
         aor.append("@");
         aor.append(*mpRegisterEventServer->getDomainName());
         aor_uri.fromString(aor, Url::AddrSpec);

         mpRegisterEventServer->
            generateContentUserInstrument(resourceId, aor, aor_uri, instrumentp, body);
      }
      else
      {
         // This is a ~~in~[instrument] URI.
         const char* instrumentp = user.data() + sizeof (URI_IN_PREFIX) - 1;
         mpRegisterEventServer->
            generateContentInstrument(resourceId, instrumentp, body);
      }         
   }
   else
   {
      // Construct the AOR.
      UtlString aor;
      Url aor_uri;
      aor.append("sip:");
      aor.append(user);
      aor.append("@");
      aor.append(*mpRegisterEventServer->getDomainName());
      aor_uri.fromString(aor, Url::AddrSpec);

      // Construct the content.
      mpRegisterEventServer->generateContentUser(resourceId, aor, aor_uri, body);
   }

   // Install it for the resource, but do not publish it, because our
   // caller will publish it.
   contentMgr->revised_publish(resourceId, eventTypeKey, eventType,
                       1, &body,
                       TRUE, TRUE);
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
      NULL, // authenicateRealm
      NULL, // authenticateDb
      NULL, // authorizeUserIds
      NULL, // authorizePasswords
      NULL, // lineMgr
      SIP_DEFAULT_RTT, // sipFirstResendTimeout
      TRUE, // defaultToUaTransactions
      -1, // readBufferSize
      OsServerTask::DEF_MAX_MSGS, // queueSize
      FALSE // bUseNextAvailablePort
      ),
   mSubscriptionMgr(SUBSCRIPTION_COMPONENT_REG, mDomainName),
   mSubscribeServer(SipSubscribeServer::terminationReasonSilent,
                    mUserAgent, mEventPublisher, mSubscriptionMgr,
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

   // Initialize the SipUserAgent.
   // Set the user-agent string.
   mUserAgent.setUserAgentHeaderProperty("sipXecs/reg-event");
   mUserAgent.start();

   // Arrange to generate default content for reg events.
   mEventPublisher.revised_publishDefault(mEventType.data(), mEventType.data(),
                                  new RegEventDefaultConstructor(this));

   // Start the SIP Subscribe Server after the initial content has
   // been published.  This ensures that early subscribers do not get
   // NOTIFYs with incomplete information.
   mSubscribeServer.enableEventType(mEventType, NULL, NULL, NULL, NULL,
         SipSubscribeServer::standardVersionCallback, TRUE);
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

// Generate and publish content for reg events for an AOR/instrument.
void RegisterEventServer::generateAndPublishContent(const UtlString& aorString,
                                                    const Url& aorUri,
                                                    const UtlString& instrument)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "RegisterEventServer::generateAndPublishContent aorString = '%s', instrument = '%s'",
                 aorString.data(), instrument.data());

   HttpBody* body;

   // Publish content for the AOR.
   generateContentUser(aorString.data(), aorString, aorUri, body);
   mEventPublisher.revised_publish(aorString, mEventType.data(), mEventType.data(),
                           1, &body,
                           TRUE, FALSE);

   if (!instrument.isNull())
   {
      // Publish content for ~~in~[instrument]@[domain].

      UtlString instrumentEntity;
      instrumentEntity.append("sip:" URI_IN_PREFIX);
      instrumentEntity.append(instrument);
      instrumentEntity.append("@");
      instrumentEntity.append(*getDomainName());

      generateContentInstrument(instrumentEntity.data(), instrument, body);
      mEventPublisher.revised_publish(instrumentEntity, mEventType.data(), mEventType.data(),
                              1, &body,
                              TRUE, FALSE);
      
      // Publish content for ~~in~[user]&[instrument]@[domain].

      UtlString userInstrumentEntity;
      userInstrumentEntity.append("sip:" URI_IN_PREFIX);
      UtlString user;
      aorUri.getUserId(user);
      userInstrumentEntity.append(user);
      userInstrumentEntity.append("&");
      userInstrumentEntity.append(instrument);
      userInstrumentEntity.append("@");
      userInstrumentEntity.append(*getDomainName());

      generateContentUserInstrument(aorString.data(), aorString, aorUri, instrument, body);
      mEventPublisher.revised_publish(userInstrumentEntity, mEventType.data(), mEventType.data(),
                              1, &body,
                              TRUE, FALSE);
   }
}

// Generate (but not publish) content for reg events for an AOR.
void RegisterEventServer::generateContentUser(const char* entity,
                                              const UtlString& aorString,
                                              const Url& aorUri,
                                              HttpBody*& body)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "RegisterEventServer::generateContentUser aorString = '%s'",
                 aorString.data());

   // Use an expiraton time of 0 to get all the registrations for the
   // AOR, including the ones that have expired but not been purged.
   ResultSet rs;
   getRegistrationDBInstance()->getUnexpiredContactsUser(aorUri,
                                                         0,
                                                         rs);

   generateContent(entity, rs, body);
}

// Generate (but not publish) content for reg events for an instrument
void RegisterEventServer::generateContentInstrument(const char* entity,
                                                    const UtlString& instrument,
                                                    HttpBody*& body)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "RegisterEventServer::generateContentInstrument instrument = '%s'",
                 instrument.data());

   // Use an expiraton time of 0 to get all the registrations for the
   // AOR, including the ones that have expired but not been purged.
   ResultSet rs;
   getRegistrationDBInstance()->getUnexpiredContactsInstrument(instrument.data(),
                                                               0,
                                                               rs);

   generateContent(entity, rs, body);
}

// Generate (but not publish) content for reg events for an AOR and instrument
void RegisterEventServer::generateContentUserInstrument(const char* entity,
                                                        const UtlString& aorString,
                                                        const Url& aorUri,
                                                        const UtlString& instrument,
                                                        HttpBody*& body)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "RegisterEventServer::generateContentUserInstrument aorString = '%s', instrument = '%s'",
                 aorString.data(), instrument.data());

   // Use an expiraton time of 0 to get all the registrations for the
   // AOR, including the ones that have expired but not been purged.
   ResultSet rs;
   getRegistrationDBInstance()->getUnexpiredContactsUserInstrument(aorUri,
                                                                   instrument.data(),
                                                                   0,
                                                                   rs);

   generateContent(entity, rs, body);
}

// Generate (but not publish) content for reg events.
void RegisterEventServer::generateContent(const char* entityString,
                                          ResultSet& rs,
                                          HttpBody*& body)
{
   unsigned long now = OsDateTime::getSecsSinceEpoch();

   // Construct the body, an empty notice for the user.
   UtlString content;
   content.append("<?xml version=\"1.0\"?>\r\n"
                  "<reginfo xmlns=\"urn:ietf:params:xml:ns:reginfo\" "
                  "xmlns:gr=\"urn:ietf:params:xml:ns:gruuinfo\" "
                  "xmlns:in=\"http://www.sipfoundry.org/sipX/schema/xml/reg-instrument-00-00\" "
                  "version=\"" VERSION_PLACEHOLDER "\" "
                  "state=\"full\">\r\n");
   content.append("  <registration aor=\"");
   XmlEscape(content, entityString);
   content.append("\" id=\"");
   XmlEscape(content, entityString);
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
      UtlString* instrument =
         dynamic_cast <UtlString*> (rowp->findValue(&RegistrationDB::gInstrumentKey));

      content.append("    <contact id=\"");
      // We key the registrations table on identity and contact URI, so
      // for the id of the <content> element, we use the concatenation of
      // AOR and contact.  We could hash these together and take 64 bits
      // if we wanted the id's to be smaller and opaque.
      XmlEscape(content, entityString);
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

      if (NULL != pathVector &&
          !pathVector->isNull() &&
          0 != pathVector->compareTo(SPECIAL_IMDB_NULL_VALUE))
      {
         content.append("      <unknown-param name=\"path\">");
         XmlEscape(content, *pathVector);
         content.append("</unknown-param>\r\n");
      }
      if (NULL != instanceId &&
          !instanceId->isNull() &&
          0 != instanceId->compareTo(SPECIAL_IMDB_NULL_VALUE))
      {
         content.append("      <unknown-param name=\"+sip.instance\">");
         XmlEscape(content, *instanceId);
         content.append("</unknown-param>\r\n");
      }
      if (NULL != gruu &&
          !gruu->isNull() &&
          0 != gruu->compareTo(SPECIAL_IMDB_NULL_VALUE))
      {
         content.append("      <gr:pub-gruu uri=\"");
         // It is a bit clunky to just prepend "sip:" onto the GRUU identity.
         // But if we were handling things properly as URIs, the gruu column
         // of the registration DB would contain the full GRUU URI already.
         UtlString tmp("sip:");
         tmp.append(*gruu);
         XmlEscape(content, tmp);
         content.append("\"/>\r\n");
      }
      if(NULL != instrument &&
         !instrument->isNull() &&
         0 != instrument->compareTo(SPECIAL_IMDB_NULL_VALUE))
      {
         content.append("      <in:instrument>");
         XmlEscape(content, *instrument);
         content.append("</in:instrument>\r\n");
      }

      content.append("    </contact>\r\n");
   }

   content.append("  </registration>\r\n");
   content.append("</reginfo>\r\n");

   // Build an HttpBody -- returned in 'body'.
   body = new HttpBody(content, strlen(content),
                       REG_EVENT_CONTENT_TYPE);
}
