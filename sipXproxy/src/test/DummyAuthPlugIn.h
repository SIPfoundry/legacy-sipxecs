// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _DUMMYAUTHPLUGIN_H_
#define _DUMMYAUTHPLUGIN_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "AuthPlugin.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

class DummyAuthPlugin : public AuthPlugin
{
  public:
   virtual ~DummyAuthPlugin();
   virtual
      AuthResult authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
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
   virtual void readConfig( OsConfigDb& configDb ){};

  protected:
  private:
   friend AuthPlugin* getAuthPlugin(const UtlString& name);
   friend class SipRouterTest;
   
   /// Constructor - private so that only the factory can call it.
   DummyAuthPlugin(const UtlString& instanceName );
   DummyAuthPlugin(const DummyAuthPlugin& nocopyconstructor);
   DummyAuthPlugin& operator=(const DummyAuthPlugin& noassignmentoperator);
   
   AuthResult mLastAuthResult;
   UtlString  mLastAuthenticatedId;
   bool       mbDenyNextRequest;
};

#endif // _DUMMYAUTHPLUGIN_H_
