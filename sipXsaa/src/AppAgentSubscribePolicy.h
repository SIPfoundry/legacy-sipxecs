//
//
// Copyright (C) 2009, 2010 Avaya, Inc certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _AppAgentSubscribePolicy_h_
#define _AppAgentSubscribePolicy_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "net/SipSubscribeServerEventHandler.h"
#include "sipXecsService/SipNonceDb.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class SipMessage;

// TYPEDEFS

//! Specialize SipSubscribeServerEventHandler to enforce Appearance Agent policies.
class AppAgentSubscribePolicy : public SipSubscribeServerEventHandler
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    //! Default Dialog constructor
    AppAgentSubscribePolicy(UtlString defaultDomain,
                       ///< our SIP domain
                       UtlString realm,
                       ///< realm to use for authentication
                       UtlString credentialDbName = "credential"
                       ///< name of the credential DB to use.
       );

    //! Destructor
    virtual
    ~AppAgentSubscribePolicy();

/* ============================ MANIPULATORS ============================== */

    //* The key to identify the resource for dialog;sla subscriptions is in
    //* the From tag, not the requestURI
    virtual UtlBoolean getKeys(const SipMessage& subscribeRequest,
                               UtlString& resourceId,
                               UtlString& eventTypeKey,
                               UtlString& eventType);

    //! Determine if the given SUBSCRIBE request is authorized to subscribe
    /*! Only allowed if 'eventlist' is supported.
     */
    virtual UtlBoolean isAuthorized(const SipMessage& subscribeRequest,
                                    SipMessage& subscribeResponse);

    virtual UtlBoolean isAuthenticated(const SipMessage & subscribeRequest,
                                       SipMessage & subscribeResponse);

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Copy constructor NOT ALLOWED
    AppAgentSubscribePolicy(const AppAgentSubscribePolicy& rRlsSubscribePolicy);

    //! Assignment operator NOT ALLOWED
    AppAgentSubscribePolicy& operator=(const AppAgentSubscribePolicy& rhs);

    //! Authentication realm to use for challenges and responses.
    UtlString mRealm;
    //! SipNonceDb to generate nonces.
    SipNonceDb mNonceDb;
    //! Time that generated nonces will be valid, in seconds.
    long mNonceExpiration;
    //! The SIP domain.
    UtlString mDefaultDomain;
    //! Name of the credentail DB to use
    UtlString mCredentialDbName;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _AppAgentSubscribePolicy_h_
