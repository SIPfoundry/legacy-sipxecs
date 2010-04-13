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

    /// Retrieve all updates for registrarName whose update number is greater than updateNumber
    int pullUpdates(
       const UtlString& registrarName,
       Int64            updateNumber,
       UtlSList&        updates);
    /**<
     * Retrieve all updates for registrarName whose update number is greater than updateNumber.
     * Return the updates in the updates list.  Each update is an object of type
     * RegistrationBinding.
     * The order of updates in the list is not specified.
     * Return the number of updates in the list.
     */

    /// Apply registry updates for a single registrar (local or peer) to the database
    Int64 applyUpdatesToDirectory(
       const UtlSList& updates,             ///< list of updates to apply
       UtlString*      errorMsg = NULL);    ///< fill in the error message on failure (may be NULL)
    /**<
     * Return the maximum update number for that registrar after applying updates, or -1
     * if there is an error.  An empty updates list is an error.
     */

    /// Get the largest update number in the local database for this registrar as primary
    Int64 getDbUpdateNumber() const;

    /// Schedule garbage collection and persistence of the registration database
    void scheduleCleanAndPersist();

    /// Garbage-collect and persist the registration database
    void cleanAndPersist();
    /**<
     * Don't call cleanAndPersist directly.  Instead call scheduleCleanAndPersist so that
     * persistence is periodic instead of immediate, for efficiency.
     */

    /// Reset the DbUpdateNumber so that the upper half is the epoch time.
    void resetDbUpdateNumberEpoch();

    /// Recover the DbUpdateNumber from the local database
    void restoreDbUpdateNumber();

    /// Return the max update number for primaryRegistrar, or zero if there are no such updates
    Int64 getMaxUpdateNumberForRegistrar(const char* primaryName) const;

    /// Return true if there is a new update to send to the peer registrar and fill in bindings
    bool getNextUpdateToSend(RegistrarPeer *peer,       ///< peer to send the update to
                             UtlSList&   bindings);     ///< fill in bindings of the update

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

    SipNonceDb mNonceDb;
    long mNonceExpiration;

    PluginHooks* mpSipRegisterPlugins;

    // The last update number assigned to a registration.  Equals zero if no
    // local registrations have been processed yet.
    UtlLongLongInt mDbUpdateNumber;

    static OsMutex sLockMutex;

    /// An additional contact to be added to all success responses, if not null.
    UtlString mAdditionalContact;

    /// Set the largest update number in the local database for this registrar as primary
    void setDbUpdateNumber(Int64 dbUpdateNumber);

    /// Validate bindings, and if all are OK then apply them to the registry db
    RegisterStatus applyRegisterToDirectory(
        const Url& toUrl,  /**< To header name-addr from the message,
                            *   which contains the AOR to register to. */
        const UtlString& instrument,
                           ///< instrument value to add to bindings
        const int timeNow, ///< base time for all expiration calculations
        const SipMessage& registerMessage, ///< message containing bindings
        RegistrationExpiryIntervals*& expiryIntervalsUsed ); ///< returns the expiry interval used to bound the expiry of the registration

    /// Update one binding for a peer registrar, or the local registrar (if peer is NULL)
    Int64 updateOneBinding(RegistrationBinding* update,
                           RegistrarPeer* peer,
                           RegistrationDB* imdb);
    /**<
     * Applies update without testing CSeq -- the caller must have
     * called RegistrationDB::isOutOfSequence and received a false
     * return.
     * Returns the max updateNumber for the registrar that is primary for this binding
     * (after the update is applied).
     * Update state variables for the primary registrar.
     */

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

    /// If replication is configured, then name of this registrar as primary
    const UtlString& primaryName() const;

    /// determine whether or not the registant is located behind a remote NAT.
    bool isRegistrantBehindNat( const SipMessage& registerRequest ) const;

};

#endif // SIPREGISTRARSERVER_H
