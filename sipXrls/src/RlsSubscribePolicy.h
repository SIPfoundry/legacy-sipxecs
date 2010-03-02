//
//
// Copyright (C) 2007, 2010 Avaya, Inc certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _RlsSubscribePolicy_h_
#define _RlsSubscribePolicy_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SipSubscribeServerEventHandler.h>
#include <sipXecsService/SipNonceDb.h>
#include <sipdb/CredentialDB.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class SipMessage;
class UtlString;
class SipPublishContentMgr;

// TYPEDEFS

//! Specialize SipSubscribeServerEventHandler to enforce RLS policies.
/*! In particular, SUBSCRIBEs that do not support 'eventlist' are to be
 *  rejected.
 */
class RlsSubscribePolicy : public SipSubscribeServerEventHandler
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    //! Default Dialog constructor
    RlsSubscribePolicy(UtlString defaultDomain,
                       ///< our SIP domain
                       UtlString realm,
                       ///< realm to use for authentication
                       UtlString credentialDbName = "credential"
                       ///< name of the credential DB to use.
       );

    //! Destructor
    virtual
    ~RlsSubscribePolicy();

/* ============================ MANIPULATORS ============================== */

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
    RlsSubscribePolicy(const RlsSubscribePolicy& rRlsSubscribePolicy);

    //! Assignment operator NOT ALLOWED
    RlsSubscribePolicy& operator=(const RlsSubscribePolicy& rhs);

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

#endif  // _RlsSubscribePolicy_h_
