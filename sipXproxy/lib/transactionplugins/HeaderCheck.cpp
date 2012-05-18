/**
 *
 *
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

#include "HeaderCheck.h"
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "utl/UtlRegex.h"
#include "net/SignedUrl.h"

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" TransactionPlugin* getTransactionPlugin(const UtlString& pluginName)
{
   return new HeaderCheck(pluginName);
}

SipRouter* HeaderCheck::_sipRouter = 0;
static bool trnRemoveAcceptLanguage = false;
static bool trnRemoveRemotePartyId = false;
static bool trnAddRecordRouteIfNotSpiral = false;
static bool trnAddPathHeaderIfRegRequest = false;
static UtlString knownUserAgents;


void HeaderCheck::readConfig( OsConfigDb& configDb )
{
  trnRemoveAcceptLanguage = configDb.getBoolean("REMOVE_ACCEPT_LANGUAGE",false);
  trnRemoveRemotePartyId = configDb.getBoolean("REMOVE_REMOTE_PARTY_ID",false);
  trnAddRecordRouteIfNotSpiral = configDb.getBoolean("INSERT_RECORD_ROUTE_AFTER_SPIRAL",false);
  trnAddPathHeaderIfRegRequest = configDb.getBoolean("INSERT_PATH_ON_REGISTER",false);
  configDb.get("INSERT_PATH_ON_REGISTER_UA_REGEX", knownUserAgents);

  if (trnRemoveAcceptLanguage)
  {
    OsSysLog::add(FAC_SIP, PRI_CRIT, "HeaderCheck::readConfig trnRemoveAcceptLanguage ENFORCED.");
  }

  if (trnRemoveRemotePartyId)
  {
    OsSysLog::add(FAC_SIP, PRI_CRIT, "HeaderCheck::readConfig trnRemoveRemotePartyId ENFORCED.");
  }

  if (trnAddRecordRouteIfNotSpiral)
  {
    OsSysLog::add(FAC_SIP, PRI_CRIT, "HeaderCheck::readConfig trnAddRecordRouteIfNotSpiral ENFORCED.");
  }

  if (trnAddPathHeaderIfRegRequest)
  {
    OsSysLog::add(FAC_SIP, PRI_CRIT, "HeaderCheck::readConfig trnAddPathHeaderIfRegRequest ENFORCED.");
  }
}


static bool isKnownUserAgent(const SipMessage& msg)
{
  static RegEx trnUserAgentRegex(knownUserAgents.data());
  UtlString userAgent;
  msg.getUserAgentField( &userAgent );
  return trnUserAgentRegex.Search(userAgent);
}

static void applyUserAgentCompatibility(SipMessage& msg)
{
  //
  // UserAgent phones sends a 406 when they receive an accept-language header.
  //
  if (trnRemoveAcceptLanguage)
  {
    if (msg.removeHeader("accept-language", 0))
    {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "HeaderCheck::handleOutputMessage::applyUserAgentCompatibility removed header Accept-Language");
    }
  }
  //
  // There is a known bug in trn phones failing to parse Remote-Party-Id and resets
  //
  if (trnRemoveRemotePartyId)
  {
    if (msg.removeHeader("remote-party-id", 0))
    {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "HeaderCheck::handleOutputMessage::applyUserAgentCompatibility removed header Remote-Party-Id");
    }
  }
}


static void trnAddRecordRouteIfnotSpiral(SipMessage& sipRequest)
{
  if (!trnAddRecordRouteIfNotSpiral || sipRequest.isResponse())
    return;

  UtlString method;
  sipRequest.getRequestMethod(&method);

  if( method.compareTo(SIP_INVITE_METHOD) != 0)
    return;

  if( !sipRequest.getHeaderValue( 0, SIP_SIPX_SPIRAL_HEADER ))
  {
    UtlString oldRecordRoute;
    if (sipRequest.getRecordRouteUri(0, &oldRecordRoute))
    {
      UtlString hostPort;
      Url oldRecordRouteUrl(oldRecordRoute);
      oldRecordRouteUrl.getHostWithPort(hostPort);
      if (hostPort == HeaderCheck::_sipRouter->getHostPort())
        return;
    }

    //
    // This request no longer spirals
    //
    // Generate the Record-Route string to be used by proxy to Record-Route requests
    // based on the route name
    UtlString recordRoute;
    Url route(HeaderCheck::_sipRouter->getHostPort());
    route.setUrlParameter("lr",NULL);
    route.toString(recordRoute);
    sipRequest.addRecordRouteUri(recordRoute);

    OsSysLog::add(FAC_SIP, PRI_DEBUG, "HeaderCheck::handleOutputMessage::trnAddRecordRouteIfnotSpiral added record-route: %s", recordRoute.data());
  }
}

static void trnAddPathHeaderIfRegisterRequest(SipMessage& sipRequest)
{
   if (!trnAddPathHeaderIfRegRequest || sipRequest.isResponse())
     return;

   UtlString method;
   sipRequest.getRequestMethod(&method);

   if( method.compareTo(SIP_REGISTER_METHOD) == 0 && isKnownUserAgent(sipRequest))
   {
     UtlString oldPath;
     if (!sipRequest.getPathUri(0, &oldPath))
     {
       Url pathUri(HeaderCheck::_sipRouter->getHostPort());
       SignedUrl::sign(pathUri);
       UtlString pathUriString;
       pathUri.toString( pathUriString );
       sipRequest.addPathUri(pathUriString);
       OsSysLog::add(FAC_SIP, PRI_DEBUG, "HeaderCheck::handleOutputMessage::trnAddPathHeaderIfRegisterRequest inserted sticky path header for REGISTER");
     }
   }
}


/// Called when SIP messages are about to be sent by proxy
void HeaderCheck::handleOutputMessage( SipMessage& message,
                                     const char* address,
                                     int port )
{
  applyUserAgentCompatibility(message);
  trnAddPathHeaderIfRegisterRequest(message);
  trnAddRecordRouteIfnotSpiral(message);
}

void HeaderCheck::announceAssociatedSipRouter( SipRouter* sipRouter )
{
  HeaderCheck::_sipRouter = sipRouter;
  HeaderCheck::_sipRouter->addSipOutputProcessor( this );
}

HeaderCheck::HeaderCheck(const UtlString& instanceName) :
  TransactionPlugin(instanceName),
  SipOutputProcessor(200)
{
  OsSysLog::add(FAC_SIP, PRI_CRIT, "HeaderCheck Transaction Plugin (%s) CREATED", instanceName.data());
}


HeaderCheck::~HeaderCheck()
{
}







