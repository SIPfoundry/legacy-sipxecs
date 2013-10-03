/**
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
 * 
 */


#include <sipx/proxy/ReproGlue.h>
#include "WSRouter.h"

const char* PLUGIN_NAME = "WebSocket Router";
const int PLUGIN_AUTH_PRIO = 10;
const int PLUGIN_IO_PRIO = 10;
const int PLUGIN_FORWARD_PRIO = 10;
EXPORT_APP_PLUGIN(new WSRouter(PLUGIN_NAME, PLUGIN_AUTH_PRIO, PLUGIN_IO_PRIO, PLUGIN_FORWARD_PRIO));

using namespace sipx::proxy;


WSRouter::WSRouter(const std::string& pluginName, int authPriority, int ioPriority, int forwardPriority) :
  UnifiedProxyPlugin(pluginName, authPriority, ioPriority, forwardPriority)
{
 
  //
  // Register the initialize function
  //
  _initialize = boost::bind(&WSRouter::initialize, this);
  
  //
  // Register the authorize function
  //
  _authorize = boost::bind(&WSRouter::authorizeAndModify,
    this, _1, _2, _3, _4, _5, _6, _7, _8);
  
  //
  // Register the forward function
  //
  _forward = boost::bind(&WSRouter::forwardRequest, this, _1, _2);
  
  //
  // Register I/O Processors
  //
  _inbound = boost::bind(&WSRouter::handleInputMessage, this,_1, _2, _3, _4);
  _outbound = boost::bind(&WSRouter::handleOutputMessage, this,_1, _2, _3, _4);
}


bool WSRouter::initialize()
{
  return true;
}

WSRouter::AuthResult WSRouter::authorizeAndModify(
    const UtlString& id,      /**  The authenticated identity of the
                                          *  request originator, if any
                                          *  (the null string if not).
                                          *  This is in the form of a SIP uri
                                          *  identity value as used in the
                                          *  credentials database (user@domain)
                                          *  without the scheme or any
                                          *  parameters.
                                          */
    const Url&  requestUri,  /// parsed target Uri
    RouteState& routeState,  /// the state for this request.  
    const UtlString& method, /// the request method
    AuthResult  priorResult, /// results from earlier plugins.
    SipMessage& request,     /// see below regarding modifying this
    bool bSpiralingRequest,  /// true if request is still spiraling through proxy
                             /// false if request is ready to be sent to target
    UtlString&  reason       /// rejection reason
  )
{
  return CONTINUE;
}
  
void WSRouter::forwardRequest(
    const Url& requestUri, /// The normalized request-uri of the SIP Message
    const SipMessage& request /// The outbound request
)
{  
}

void WSRouter::handleInputMessage(SipMessage& message, const char* address, int port, bool buffered)
{
}
  
void WSRouter::handleOutputMessage(SipMessage& message, const char* address, int port, bool buffered)
{
}