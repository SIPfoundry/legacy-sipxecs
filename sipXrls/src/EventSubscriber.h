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

#ifndef SUBSCRIPTIONEVENT_H_INCLUDED
#define	SUBSCRIPTIONEVENT_H_INCLUDED


#include <string>
#include <iostream>
#include <sstream>
#include <rutil/SharedPtr.hxx>
#include <resip/stack/StackThread.hxx>
#include <resip/stack/SipMessage.hxx>
#include <resip/stack/OctetContents.hxx>
#include <resip/dum/SubscriptionHandler.hxx>
#include <resip/dum/RegistrationHandler.hxx>
#include <resip/dum/Handles.hxx>
#include <resip/dum/DumThread.hxx>
#include <resip/dum/MasterProfile.hxx>
#include <resip/dum/ClientAuthManager.hxx>
#include <resip/dum/ClientSubscription.hxx>
#include <resip/stack/SipStack.hxx>

#include "ResipLogger.h"
#include "SubscriptionDialog.h"

#include "os/OsServiceOptions.h"

namespace resip {

class EventSubscriber  : public resip::ClientSubscriptionHandler, boost::noncopyable
{
protected:
  class SubscriptionEvent : public resip::DumCommand
  {
  public:
    SubscriptionEvent(EventSubscriber& client,
      const resip::Uri& aor,
      const resip::Uri& requestUri,
      const resip::Uri& route,
      int refreshInterval);

    void executeCommand();

    resip::Message* clone() const;

    std::ostream& encode(std::ostream& strm) const;

    std::ostream& encodeBrief(std::ostream& strm) const;
  protected:
    EventSubscriber& _client;
    resip::Uri _aor;
    int _refreshInterval;
    resip::Uri _requestUri;
    resip::Uri _route;
    friend class EventSubscriber;
  };

  OsServiceOptions& _options;
  resip::Security* _security;
  resip::SipStack _stack;
  resip::StackThread _stackThread;
  SubscriptionDUM _dum;
  resip::DumThread _dumThread;
  resip::SharedPtr<resip::MasterProfile> _profile;
  Token _event;
  unsigned short _port;
  ResipLogger _reproLogger;
  std::string _route;
  int _refreshInterval;
public:

  EventSubscriber(OsServiceOptions& options);

  ~EventSubscriber();

  void run();

  void stop();

  void subscribe(SubscriptionEvent& eventData);

  void subscribe();

  const OctetContents* toGenericContents(const SipMessage& notify);

  void logEvent(ClientSubscriptionHandle h, const SipMessage& notify, bool outOfOrder);

  void onNewSubscription(ClientSubscriptionHandle h, const SipMessage& notify);

  void onUpdatePending(ClientSubscriptionHandle h, const SipMessage& notify, bool outOfOrder);

  void onUpdateActive(ClientSubscriptionHandle h, const SipMessage& notify, bool outOfOrder);

  void onUpdateExtension(ClientSubscriptionHandle h, const SipMessage& notify, bool outOfOrder);

  int onRequestRetry(ClientSubscriptionHandle, int retrySeconds, const SipMessage& notify);

  void onTerminated(ClientSubscriptionHandle handle, const SipMessage* msg);

  bool loadFromResourceFile(const std::string& path, const std::string& domain, std::vector<std::string>& resourses);
};

} // resip



#endif	/// SUBSCRIPTIONEVENT_H_INCLUDED

