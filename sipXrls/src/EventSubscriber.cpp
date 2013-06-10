
/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */


#include "EventSubscriber.h"

#include <xmlparser/tinystr.h>
#include <xmlparser/ExtractContent.h>
#include <xmlparser/TiXmlUtlStringWriter.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlString.h>
#include <utl/XmlContent.h>


namespace resip {


EventSubscriber::SubscriptionEvent::SubscriptionEvent(EventSubscriber& client,
  const resip::Uri& aor,
  const resip::Uri& requestUri,
  const resip::Uri& route,
  int refreshInterval) :
  _client(client),
  _aor(aor),
  _refreshInterval(refreshInterval),
  _requestUri(requestUri),
  _route(route)
{
}

void EventSubscriber::SubscriptionEvent::executeCommand()
{
  _client.subscribe(*this);
}

resip::Message* EventSubscriber::SubscriptionEvent::clone() const
{
  return new SubscriptionEvent(_client, _aor, _requestUri, _route, _refreshInterval);
}

std::ostream& EventSubscriber::SubscriptionEvent::encode(std::ostream& strm) const
{
  strm << "Add event watcher " << _aor;
  return strm;
}

std::ostream& EventSubscriber::SubscriptionEvent::encodeBrief(std::ostream& strm) const
{
  return encode(strm);
}



EventSubscriber::EventSubscriber(OsServiceOptions& options) :
  _options(options),
  _security(0),
  _stack(_security),
  _stackThread(_stack),
  _dum(_stack),
  _dumThread(_dum),
  _profile(new MasterProfile),
  _event("dialog"),
  _port(5060),
  _refreshInterval(300)
{

  //
  // Initialize logging
  //
  std::string logFile;
  options.getOption("log-file", logFile, "sipxsub.log");
  Log::initialize(Log::Cout, Log::Debug, "sipxsub", logFile.c_str(), &_reproLogger);


  _profile->addSupportedMethod(NOTIFY);
  _profile->addAllowedEvent(_event);
  _profile->validateAcceptEnabled() = false;
  _profile->validateContentEnabled() = false;


  //
  // Set the authenticator
  //
  std::string authDomain;
  std::string authUser;
  std::string authPassword;
  options.getOption("auth-domain", authDomain);
  options.getOption("auth-user", authUser);
  options.getOption("auth-pass", authPassword);
  _route = "sip:";
  _route += authDomain;
  _route += ";lr";
  if (!authDomain.empty() && !authUser.empty() && !authPassword.empty())
    _profile->setDigestCredential(authDomain.c_str(), authUser.c_str(), authPassword.c_str());

  std::string localUri;
  options.getOption("local-uri", localUri);

  if (localUri.empty())
  {
    std::cerr << "Error:  You must provide a values for local-uri, resource-uri and proxy-uri!!!" << std::endl;
    _options.displayUsage(std::cerr);
    _exit(1);
  }

  Data unparsedAor(localUri.c_str());
  NameAddr from(unparsedAor);
  _profile->setDefaultFrom(from);

  Token eventlist("eventlist");
  _profile->addSupportedOptionTag(eventlist);

  std::string userAgent;
  options.getOption("user-agent", userAgent, "sipxsub");
  _profile->setUserAgent(userAgent.c_str());


  int sipPort = 5060;
  options.getOption("sip-port", sipPort, 5060);
  _port = sipPort;


  std::string resource;
  options.getOption("resource-type", resource, "dialog");
  _event = Token(resource.c_str());

  _dum.setMasterProfile(_profile);


  std::auto_ptr<resip::ClientAuthManager> clam(new resip::ClientAuthManager());
  _dum.setClientAuthManager(clam);
  _dum.addClientSubscriptionHandler(_event.value(), this);


  std::auto_ptr<resip::AppDialogSetFactory> dsf(new SubscribeAppDialogSetFactory());
  _dum.setAppDialogSetFactory(dsf);
}

EventSubscriber::~EventSubscriber()
{
}

void EventSubscriber::run()
{
  _dum.addTransport(UDP, _port);
  _dum.addTransport(TCP, _port);
  _stackThread.run();
  _dumThread.run();
}

void EventSubscriber::stop()
{
  OS_LOG_NOTICE(FAC_SIP, "EventSubscriber::stop - DUM");
  _dumThread.shutdown();
  OS_LOG_NOTICE(FAC_SIP, "EventSubscriber::stop - SIP Stack");
  _stackThread.shutdown();
  OS_LOG_NOTICE(FAC_SIP, "EventSubscriber::stop - DUM join");
  _dumThread.join();
  OS_LOG_NOTICE(FAC_SIP, "EventSubscriber::stop - SIP Stack join");
  _stackThread.join();
  OS_LOG_NOTICE(FAC_SIP, "Session State Watcher TERMINATED.");
}

void EventSubscriber::subscribe(SubscriptionEvent& eventData)
{
  //
  // This is where the subscription is formed
  //
  SharedPtr<SipMessage> sub = _dum.makeSubscription(resip::NameAddr(eventData._aor), _event.value(), eventData._refreshInterval);
  //
  // Set the request-ling
  //
  sub->header(h_RequestLine).uri() = eventData._requestUri;

  //
  // if event package is dialog, add event list to supported packages
  //
  //Token eventlist("eventlist");
  //sub->header(h_Supporteds).push_back(eventlist);

  //
  // Insert the route set
  //
  NameAddr routeAddr;
  routeAddr.uri() = eventData._route;
  NameAddrs sRoute;
  sRoute.push_back(routeAddr);
  sub->header(h_Routes) = sRoute;

  OS_LOG_INFO(FAC_SIP, "EventSubscriber::SubscriptionEvent event handled ... ");

  _dum.send(sub);
}




bool EventSubscriber::loadFromResourceFile(const std::string& path, const std::string& domain, std::vector<std::string>& resources)
{
  TiXmlDocument document;
  TiXmlNode* lists_node;
  if (document.LoadFile(path.c_str()) &&
    (lists_node = document.FirstChild("lists")) != 0 &&
    lists_node->Type() == TiXmlNode::ELEMENT)
  {
    for (TiXmlNode* list_node = 0;
      (list_node = lists_node->IterateChildren("list",
      list_node));)
    {
      if (list_node->Type() == TiXmlNode::ELEMENT)
      {
        TiXmlElement* list_element = list_node->ToElement();
        // Process the 'user' attribute.
        const char* user_attribute = list_element->Attribute("user");
        if (user_attribute && *user_attribute != '\0')
        {          // User missing or null.
          std::ostringstream strm;
          strm << "sip:" << user_attribute << "@" << domain;
          resources.push_back(strm.str());
        }
      }
    }
  }
  else
  {
    return false;
  }
  
  return true;
}

void EventSubscriber::subscribe()
{
  std::string requestUri_;
  std::string route_;
  std::string resourceFile_;
  int refreshInterval = 300;

  _options.getOption("resource-uri", requestUri_);
  _options.getOption("proxy-uri", route_);
  _options.getOption("resource-file", resourceFile_);

  if (route_.empty())
  {
    std::cerr << "Error: You must provide a value for proxy-uri!!!" << std::endl;
    _options.displayUsage(std::cerr);
    _exit(1);
  }

  if (refreshInterval == -1)
    refreshInterval = _refreshInterval;

  std::vector<std::string> resources;
  if (!requestUri_.empty())
  {
    resources.push_back(requestUri_);
  }
  else if (!resourceFile_.empty())
  {
    std::string domain;
    _options.getOption("auth-domain", domain);

    if (domain.empty())
    {
      std::cerr << "Error: You must provide a value for auth-domain!!!" << std::endl;
      _options.displayUsage(std::cerr);
      _exit(1);
    }

    if (!loadFromResourceFile(resourceFile_, domain, resources))
    {
       std::cerr << "Error: Unable to load " << resourceFile_ << " !!!" << std::endl;
      _options.displayUsage(std::cerr);
      _exit(1);
    }
  }
  else
  {
    std::cerr << "Error: You must provide a value for resource-uri or resource-file!!!" << std::endl;
    _options.displayUsage(std::cerr);
    _exit(1);
  }

  for (std::vector<std::string>::iterator iter = resources.begin(); iter != resources.end(); iter++)
  {

    Data rawRuri(*iter);
    Data rawRoute(route_);


    resip::Uri requestUri;
    resip::Uri route;

    try { requestUri = resip::Uri(rawRuri);} catch(...){};
    try { route = resip::Uri(rawRoute);} catch(...){};

    OS_LOG_INFO(FAC_SIP, "Sendign subscription "
            << " URI: " << *iter
            << " Route: " << route_
            << " Interval: " << refreshInterval);

    _dum.post(new SubscriptionEvent(*this, requestUri, requestUri, route, refreshInterval));
  }
}

const OctetContents* EventSubscriber::toGenericContents(const SipMessage& notify)
{
  const OctetContents* generic = dynamic_cast<const OctetContents*>(notify.getContents());
  return generic;
}

void EventSubscriber::logEvent(ClientSubscriptionHandle h, const SipMessage& notify, bool outOfOrder)
{

  std::ostringstream dialogId;
  dialogId << h->getDialogId().getLocalTag().c_str() << "-" << h->getDialogId().getRemoteTag();
  Data aor = notify.header(h_From).uri().getAorNoPort();

  if (outOfOrder)
  {
    OS_LOG_INFO(FAC_SIP, "::logEvent - Ignoring out of order NOTIFY from " << aor.c_str());
    return;
  }

  Data contact;
  ParserContainer<NameAddr> contactList(notify.header(h_Contacts));
  ParserContainer<NameAddr>::iterator i(contactList.begin());
  ParserContainer<NameAddr>::iterator iEnd(contactList.end());
  for (; i != iEnd; ++i)
  {
    if (!i->isWellFormed())
    {
      contact = i->uri().getAor();
      break;
    }
  }

  const OctetContents* generic = toGenericContents(notify);

  if (generic)
  {
    OS_LOG_DEBUG(FAC_SIP, "::logEvent - NOTIFY"
            << " aor: " << aor
            << " contact: " << contact
            << " dialog-id: " << dialogId.str()
            << " contents: " << generic->octets().c_str());
  }
}

void EventSubscriber::onNewSubscription(ClientSubscriptionHandle h, const SipMessage& notify)
{
  logEvent(h, notify, false);
}

void EventSubscriber::onUpdatePending(ClientSubscriptionHandle h, const SipMessage& notify, bool outOfOrder)
{
  h->acceptUpdate();
  logEvent(h, notify, outOfOrder);
}

void EventSubscriber::onUpdateActive(ClientSubscriptionHandle h, const SipMessage& notify, bool outOfOrder)
{
  h->acceptUpdate();
  logEvent(h, notify, outOfOrder);
}

void EventSubscriber::onUpdateExtension(ClientSubscriptionHandle h, const SipMessage& notify, bool outOfOrder)
{
   h->acceptUpdate();
   logEvent(h, notify, outOfOrder);
}

int EventSubscriber::onRequestRetry(ClientSubscriptionHandle, int retrySeconds, const SipMessage& notify)
{
   return -1;
}

void EventSubscriber::onTerminated(ClientSubscriptionHandle handle, const SipMessage* msg)
{
  std::ostringstream dialogId;
  dialogId << handle->getDialogId().getLocalTag().c_str() << "-" << handle->getDialogId().getRemoteTag();

  std::string aor;
  if (handle.isValid() && handle->peerAddr().uri().isWellFormed())
  {
    aor = handle->peerAddr().uri().scheme().c_str();
    aor += ":";
    aor += handle->peerAddr().uri().getAorNoPort().c_str();
  }

  OS_LOG_INFO(FAC_SIP, "::onTerminated removed " << aor << " from active watch list");

  std::cout << "Subscription to " << aor << " TERMINATED" << std::endl;

  _exit(0);
}


} // resip





