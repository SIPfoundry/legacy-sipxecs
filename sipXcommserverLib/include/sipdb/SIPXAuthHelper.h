//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef SIPXAUTHHELPER_H
#define SIPXAUTHHELPER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "sipdb/SIPXAuthHelper.h"

// DEFINES
#define MD5_DIGEST_LENGTH 32
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class AliasDB;
class CredentialDB;
class ExtensionDB;
class PermissionDB;

/**
 * Common helper utility for the MailboxManager CGI and the
 *
 * @author John P. Coffey
 * @version 1.0
 */
class SIPXAuthHelper
{
public:
    /** singleton */
    static SIPXAuthHelper* getInstance();

    /**
     * Virtual Destructor
     */
    virtual ~SIPXAuthHelper();

    /**
     * checks the users credentials
     */
    UtlBoolean isAuthorizedUser (
        const UtlString& loginString,
        const UtlString& loginPassToken,
        const UtlString& domainName,
        const UtlString& realmName,
        const UtlBoolean& checkPermissions,
        UtlString& rContactUserID,
        UtlString& rContactDomain,
        UtlString& rErrorLog) const;
private:
    /** Ctor */
    SIPXAuthHelper();

    UtlBoolean comparePassToken (
       const UtlString& userOrExtensionAtOptDomain,
       const UtlString& loginPassToken,
       const UtlString& realmName,
       const UtlString& dbPassToken,
       const UtlString& dbAuthType) const;


    // Singleton and Serialization mutex
    static OsMutex sLockMutex;

    static SIPXAuthHelper* spInstance;
};

#endif // SIPXAUTHHELPER_H
