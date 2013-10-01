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


#ifndef WSROUTER_H_INCLUDED
#define	WSROUTER_H_INCLUDED


#include <sipxproxy/UnifiedProxyPlugin.h>


class WSRouter : public UnifiedProxyPlugin
{
  public:
    WSRouter(const std::string& pluginName, int authPriority, int ioPriority, int forwardPriority);
    
    bool initialize();
    /// This method is called right after registration of the plugin.
    
    AuthResult authorizeAndModify(
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
  );
  
  void forwardRequest(
      const Url& requestUri, /// The normalized request-uri of the SIP Message
      const SipMessage& request /// The outbound request
  );
  /// This function is called after the internal forwarding rules has been evaluated.
  /// This would allow registered plug-ins to add custom forwarding rule on
  /// top of the sipx internal rules.
  
  void handleInputMessage(SipMessage& message, const char* address, int port, bool buffered);
  /// This handler would get a glimpse of all incoming SIP Message.
  /// Implementation can modify the SIP Message prior to transaction processing.
  ///
  
  void handleOutputMessage(SipMessage& message, const char* address, int port, bool buffered);
  /// This handler would get a glimpse of all outgojng SIP Message.
  /// Implementation can modify the SIP Message prior to sending the SIP Message.
  ///
};


#endif	/* WSROUTER_H */

