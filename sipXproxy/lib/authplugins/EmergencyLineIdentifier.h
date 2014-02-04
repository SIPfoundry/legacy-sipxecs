// Copyright (c) eZuce, Inc. All rights reserved.
// Contributed to SIPfoundry under a Contributor Agreement
//
// This software is free software; you can redistribute it and/or modify it under
// the terms of the Affero General Public License (AGPL) as published by the
// Free Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
// details.

#ifndef EMERGENCYLINEIDENTIFIER_H_INCLUDED
#define	EMERGENCYLINEIDENTIFIER_H_INCLUDED


#include <sipxproxy/AuthPlugin.h>

extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

class EmergencyLineIdentifier : public AuthPlugin
{
protected:
  EmergencyLineIdentifier(const UtlString& instanceName);
  friend AuthPlugin* getAuthPlugin(const UtlString& name);

public:
  virtual ~EmergencyLineIdentifier();

  virtual void readConfig(OsConfigDb& configDb);

  virtual AuthResult authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
                   *   request originator, if any (the null
                   *   string if not).
                   *   This is in the form of a SIP uri
                   *   identity value as used in the
                   *   credentials database (user@domain)
                   *   without the scheme or any parameters.
                   */
    const Url&  requestUri, ///< parsed target Uri
    RouteState& routeState, ///< the state for this request.
    const UtlString& method,///< the request method
    AuthResult  priorResult,///< results from earlier plugins.
    SipMessage& request,    ///< see AuthPlugin regarding modifying
    bool bSpiralingRequest, ///< request spiraling indication
    UtlString&  reason      ///< rejection reason
  );

  bool getInstrument(const SipMessage& message, std::string& intrument);
  ///
  /// Returns true if the intrument Id is found in the Digest authenticator header of the SIP message
};


#endif	/// EMERGENCYLINEIDENTIFIER_H_INCLUDED

