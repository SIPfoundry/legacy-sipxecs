//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREGISTRARSERVER_H
#define SIPREGISTRARSERVER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "os/OsServerTask.h"
#include "sipXecsService/SipNonceDb.h"
#include "utl/PluginHooks.h"
#include "sipdb/RegExpireThread.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;
class SipRegistrar;
class SipUserAgent;
class PluginHooks;
class SyncRpc;
class RegistrationExpiryIntervals;

/**
 * The Registrar Server is responsible for registering and unregistering
 * phones.  The servertask also looks up contacts for invite URI's. These
 * contacts are taken from the Url Mapping rules and also from the
 * registration database.
 */
class SipRegistrarServer : public OsServerTask
{
public:
    /// Construct the thread to process REGISTER requests.
    SipRegistrarServer(SipRegistrar& registrar);
    /**<
     * Defer init to the initialize method to allow the SipRegistrarServer object
     * to be accessed before the associated thread has been started.
     */

    /// Initialize the Registration Server
    void initialize(OsConfigDb*   configDb,        ///< Configuration parameters
                    SipUserAgent* pSipUserAgent    ///< User Agent to use when sending responses
    );

    virtual ~SipRegistrarServer();

    /**
     * Registration Status values
     */
    enum RegisterStatus
    {
        REGISTER_SUCCESS = 0,           ///< contacts updated
        REGISTER_LESS_THAN_MINEXPIRES , ///< requested duration to short
        REGISTER_INVALID_REQUEST,       ///< some other error
        REGISTER_FORBIDDEN,             ///< authenticated id not valid for AOR
        REGISTER_NOT_FOUND,             ///< no contacts match AOR
        REGISTER_OUT_OF_ORDER,          ///< newer data already in registry database
        REGISTER_QUERY                  ///< request is a valid query for current contacts
    };

protected:
    struct RegistrationExpiryIntervals
    {
       int mMinExpiresTime;   // Minimum registration expiry value in seconds
       int mMaxExpiresTime;   // Maximum registration expiry value in seconds
    };

    SipRegistrar& mRegistrar;
    UtlBoolean mIsStarted;
    SipUserAgent* mSipUserAgent;
    RegistrationExpiryIntervals mNormalExpiryIntervals; // registration expiry intervals for non-NATed UAs
    RegistrationExpiryIntervals mNatedExpiryIntervals;  // registration expiry intervals for NATed UAs
    bool mUseCredentialDB;
    UtlString mRealm;
    UtlBoolean mSendExpiresInResponse;
    UtlBoolean mSendAllContactsInResponse;
    RegExpireThread _expireThread;

    SipNonceDb mNonceDb;
    long mNonceExpiration;

    PluginHooks* mpSipRegisterPlugins;

    // The last update number assigned to a registration.  Equals zero if no
    // local registrations have been processed yet.

#if 0
// This is no longer needed because all mongo operations are thread-safe
    static OsMutex sLockMutex;
#endif

    /// An additional contact to be added to all success responses, if not null.
    UtlString mAdditionalContact;

    /// Validate bindings, and if all are OK then apply them to the registry db
    RegisterStatus applyRegisterToDirectory(
        const Url& toUrl,  /**< To header name-addr from the message,
                            *   which contains the AOR to register to. */
        const UtlString& instrument,
                           ///< instrument value to add to bindings
        const int timeNow, ///< base time for all expiration calculations
        const SipMessage& registerMessage, ///< message containing bindings
        RegistrationExpiryIntervals*& expiryIntervalsUsed ); ///< returns the expiry interval used to bound the expiry of the registration



    // Process a single REGISTER request
    UtlBoolean handleMessage( OsMsg& eventMessage );

    /// Check authentication for REGISTER request
    UtlBoolean isAuthorized(const Url& toUri, ///< AOR from the message
                            UtlString& instrument,
                            ///< (out) the instrument value found in the authentication header
                            const SipMessage& message, ///< REGISTER message
                            SipMessage& responseMessage /// response for challenge
                            );

    /**<
     * @return
     * - true if request is authenticated as user for To address
     * - false if not (responseMessage is then set up as a challenge)
     */


    /// determine whether or not the registant is located behind a remote NAT.
    bool isRegistrantBehindNat( const SipMessage& registerRequest ) const;
};

#endif // SIPREGISTRARSERVER_H
