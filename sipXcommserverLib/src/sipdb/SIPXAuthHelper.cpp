//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "net/Url.h"
#include "net/HttpMessage.h"
#include "net/NetMd5Codec.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "sipdb/ExtensionDB.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/SIPXAuthHelper.h"

// STATIC INITIALIZERS
SIPXAuthHelper* SIPXAuthHelper::spInstance = NULL;
OsMutex         SIPXAuthHelper::sLockMutex (OsMutex::Q_FIFO);

/* ============================ CREATORS ================================== */

SIPXAuthHelper::SIPXAuthHelper()
{}

SIPXAuthHelper::~SIPXAuthHelper()
{
    OsLock lock( sLockMutex );
    // force an imdb close
    delete SIPDBManager::getInstance();
    spInstance = NULL;
}

SIPXAuthHelper* SIPXAuthHelper::getInstance()
{
    // Critical Section here
    OsLock lock( sLockMutex );

    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new SIPXAuthHelper();
    }
    return spInstance;
}

UtlBoolean
SIPXAuthHelper::isAuthorizedUser (
    const UtlString& loginString,
    const UtlString& loginPassToken,
    const UtlString& realmName,
    const UtlString& domainName,
    const UtlBoolean& checkPermissions,
    UtlString& rContactUserID,
    UtlString& rContactDomain,
    UtlString& rErrorLog) const
{
    OsStatus result = OS_SUCCESS;

    UtlString userOrExtensionAtOptDomain(loginString);
    ssize_t sipIndex = userOrExtensionAtOptDomain.index("sip:");
    if ( sipIndex != UTL_NOT_FOUND )
    {
        // see if we're being passed in a full URL in which
        // case strip it down to its identity, discarding the display name
        if ( sipIndex > 0 )
        {
            Url loginUrl ( userOrExtensionAtOptDomain );
            loginUrl.getIdentity( userOrExtensionAtOptDomain );
        } else
        {
            // sip: is in first position
            userOrExtensionAtOptDomain =
                userOrExtensionAtOptDomain( 4, userOrExtensionAtOptDomain.length() - 4 );
        }
    }

    // we're going to need this irrespective of whether
    // the user is logging in via mailboxIdentity or extension
    ResultSet extensions;

    // Search the credentials database for a
    // match against the userid column
    UtlString dbRealm, dbAuthType, dbPassToken;
    Url mailboxUrl;
    bool keepsearching = TRUE;
    // Infinite loop detector
    int loopCount = 0;
    while ( keepsearching && ( result != OS_FAILED ) && (loopCount < 3) )
    {
        // first try searching the credentials database using the login string
        UtlBoolean credentialRecordFound =
            CredentialDB::getInstance()->getUserPin (
                userOrExtensionAtOptDomain, // IN
                realmName,                  // IN
                mailboxUrl,                 // OUT
                dbPassToken,                // OUT
                dbAuthType );               // OUT

        if ( !credentialRecordFound )
        {
            rErrorLog += (UtlString) "SIPXAuthHelper::isAuthorizedUser - " +
                (UtlString) "unable to find userOrExtension-" + userOrExtensionAtOptDomain +
                " in Credential DB\n";
            // Since userid does not expect any domain name associated with it, strip the domain off
            // and try again
            ssize_t atIndex = userOrExtensionAtOptDomain.index("@");
            if ( atIndex != UTL_NOT_FOUND )
            {
                userOrExtensionAtOptDomain = userOrExtensionAtOptDomain(0, atIndex);
            }

            rErrorLog += (UtlString) "SIPXAuthHelper::isAuthorizedUser -" +
                (UtlString)" retrying credentials using - " + userOrExtensionAtOptDomain + (UtlString)"\n";
            // search again adding with a realm in the search
            credentialRecordFound =
                CredentialDB::getInstance()->getUserPin (
                    userOrExtensionAtOptDomain, // IN
                    realmName,                  // IN
                    mailboxUrl,                 // OUT
                    dbPassToken,                // OUT
                    dbAuthType );               // OUT
        }

        if ( credentialRecordFound )
        {
            rErrorLog +=
                (UtlString) "SIPXAuthHelper::isAuthorizedUser -" +
                (UtlString)" Found Credential record for -" + userOrExtensionAtOptDomain +
                (UtlString)" mailboxUrl " + mailboxUrl.toString() + (UtlString)"\n";

            // Fill out the return values
            mailboxUrl.getUserId(rContactUserID);
            mailboxUrl.getHostAddress(rContactDomain);

            // Exit the search loop
            keepsearching = FALSE;

            if ( checkPermissions )
            {
                // found a match! now check for voicemail permissions
                // against the mailboxUrl that was returned above
                ResultSet permissions;
                PermissionDB::getInstance()->
                    getPermissions( mailboxUrl, permissions );

                if ( permissions.getSize() > 0 )
                {
                    // Ensure the user has the right permissions before proceeding
                    UtlBoolean permissionFound = FALSE;

                    UtlString permissionKey ("permission");

                    for ( int i=0; i<permissions.getSize(); i++ )
                    {
                        UtlHashMap record;
                        permissions.getIndex( i, record );
                        UtlString permission = *((UtlString*)record.findValue(&permissionKey));

                        // AutoAttendant permission is used  here to allow user record name for dialbyname if its
                        // Voicemail permission is not enabled.
                        if ( permission.compareTo( "Voicemail", UtlString::ignoreCase )==0 ||
                             permission.compareTo( "AutoAttendant", UtlString::ignoreCase )==0 )
                        {
                            permissionFound = TRUE;

                            // unless it is digest encoded
                            if ( dbAuthType.compareTo("DIGEST", UtlString::ignoreCase) == 0 )
                            {
                                if (!comparePassToken (userOrExtensionAtOptDomain,
                                                      loginPassToken,
                                                      realmName,
                                                      dbPassToken,
                                                      dbAuthType))  result = OS_FAILED;
                            }
                            keepsearching = FALSE;
                            // break out of for loop
                            break;
                        }
                    }

                    // it is possible that even though there is an uri defined
                    // and the passwords match, the user may have insufficient priveleges
                    if( permissionFound == FALSE )
                    {
                        // Write to the log and exit the loop.
                        rErrorLog +=
                            (UtlString) "SIPXAuthHelper::isAuthorizedUser - " +
                            (UtlString)"Voicemail Permission missing for - " + mailboxUrl.toString() +
                            (UtlString)"\n";
                        result = OS_FAILED;
                    }
                } else // we have a credential match but no voicemail permission
                {
                    // Write to the log and exit the loop.
                    rErrorLog +=
                        (UtlString)"SIPXAuthHelper::isAuthorizedUser - " +
                        (UtlString)"No Permissions for - " + mailboxUrl.toString() +
                        (UtlString)"\n";
                    result = OS_FAILED;
                }
            }
            else if (!comparePassToken (userOrExtensionAtOptDomain,
                  loginPassToken,
                  realmName,
                  dbPassToken,
                  dbAuthType))
            {
                // Write to the log and exit the loop.
                rErrorLog +=
                    (UtlString) "SIPXAuthHelper::isAuthorizedUser - " +
                    (UtlString)"password does not match for - " + mailboxUrl.toString() +
                    (UtlString)"\n";
               result = OS_FAILED;
            }
        } else // this may be an alias or a mailbox extension
        {
            rErrorLog += (UtlString) "SIPXAuthHelper::isAuthorizedUser - " +
                (UtlString) "failed to find - " + userOrExtensionAtOptDomain + (UtlString)" in Credential DB\n";

            rErrorLog += (UtlString) "SIPXAuthHelper::isAuthorizedUser - " +
                (UtlString) "Searching the ExtensionDB for a match for - " + loginString + (UtlString)"\n";

            // search for a mailbox URL (note that the sip: has
            // been stripped from the userOrExtensionAtOptDomain string
            UtlBoolean mailboxUrlFound =
                ExtensionDB::getInstance()->
                    getUri ( userOrExtensionAtOptDomain, mailboxUrl );

            if ( !mailboxUrlFound )
            {
                // searching using the userOrExtensionAtOptDomain did
                // not work, search again (making sure to add/remove
                // the domain portion)
                if ( userOrExtensionAtOptDomain.index("@") == UTL_NOT_FOUND )
                {
                    userOrExtensionAtOptDomain = userOrExtensionAtOptDomain + "@" + domainName;
                } else if ( loginString.index( domainName ) != UTL_NOT_FOUND )
                {
                    userOrExtensionAtOptDomain =
                        userOrExtensionAtOptDomain(0,
                        userOrExtensionAtOptDomain.index(domainName) -1 );
                }

                rErrorLog += (UtlString) "SIPXAuthHelper::isAuthorizedUser - " +
                    (UtlString) "Retrying with/(out) realm - " + userOrExtensionAtOptDomain +
                    " in ExtensionDB\n";

                // search again adding with a realm in the search
                mailboxUrlFound  =
                    ExtensionDB::getInstance()->getUri (
                        userOrExtensionAtOptDomain, mailboxUrl );

                if (!mailboxUrlFound)
                {
                    // Write to the log and exit the loop.
                    rErrorLog +=
                        "SIPXAuthHelper::isAuthorizedUser - "
                        "No contact found in extensions, Searching AliasDB for Unique Row\n";

                    // Ensure that we construct a valid Url to search the AliasDB
                    // Aliases have always had a full user@domain
                    Url aliasIdentityUrl;
                    if (loginString.index("@") != UTL_NOT_FOUND)
                        aliasIdentityUrl = loginString;
                    else
                        aliasIdentityUrl = loginString + "@" + domainName;

                    UtlString contactKey ("contact");
                    ResultSet aliasContacts;
                    AliasDB::getInstance()->
                        getContacts( aliasIdentityUrl, aliasContacts );
                    int numRows = aliasContacts.getSize();
                    if ( numRows != 1 )
                    {
                        if ( numRows == 0 )
                        {
                            rErrorLog +=
                                "SIPXAuthHelper::isAuthorizedUser - ERROR: Failed to find an alias match for: [" +
                                aliasIdentityUrl.toString() + "] in AliasDB\n";
                        } else
                        {
                            rErrorLog +=
                                "SIPXAuthHelper::isAuthorizedUser - ERROR: Multiple aliases for: [" +
                                aliasIdentityUrl.toString() + "] in AliasDB\n";
                        }
                        result = OS_FAILED;
                    } else // Successful match
                    {
                        UtlHashMap record;
                        aliasContacts.getIndex(0, record);
                        mailboxUrl = *((UtlString*)record.findValue(&contactKey));
                    }
                } else  // mailboxUrl is now valid
                {
                    // update the userOrExtensionAtOptDomain
                    CredentialDB::getInstance()->
                        getUserPin (
                            mailboxUrl,                  // IN
                            realmName,                   // IN
                            userOrExtensionAtOptDomain,  // OUT
                            dbPassToken,                 // OUT
                            dbAuthType );                // OUT
                }
            } else
            {
                rErrorLog += (UtlString) "SIPXAuthHelper::isAuthorizedUser - " +
                    (UtlString) "Found entry in Extensions for -" + userOrExtensionAtOptDomain +
                    (UtlString)" returns " + mailboxUrl.toString() + (UtlString)"\n";
                // found the mailbox url associated with the extension now
                // we need to query the userid from the DB and try again
                if ( !CredentialDB::getInstance()->getUserPin (
                        mailboxUrl,                 // IN
                        realmName,                  // IN
                        userOrExtensionAtOptDomain, // OUT update this for the next time round the loop
                        dbPassToken,                // OUT
                        dbAuthType ) )              // OUT
                {
                    // Write to the log and exit the loop.
                    rErrorLog += "SIPXAuthHelper::isAuthorizedUser - FAILED - to find entry in Credential DB for " + mailboxUrl.toString();
                    result = OS_FAILED;
                }
            }
        }
        loopCount += 1;
    }

    // See if we've exceeded the loop count, which could happen if the IMDB tables
    // have a recurcive relationship, extension's contact URI is not in contacts for ex.
    if ( loopCount > 2 )
    {
        rErrorLog += (UtlString) "SIPXAuthHelper::isAuthorizedUser - ERROR ExtensionDB does not have a valid CredentialDB Contact!\n";
        result = OS_FAILED;
    }
    return(result == OS_SUCCESS);
}


UtlBoolean
SIPXAuthHelper::comparePassToken (
    const UtlString& userOrExtensionAtOptDomain,
    const UtlString& loginPassToken,
    const UtlString& realmName,
    const UtlString& dbPassToken,
    const UtlString& dbAuthType) const
{
   UtlBoolean result = FALSE;
    // unless it is digest encoded
    if ( dbAuthType.compareTo("DIGEST", UtlString::ignoreCase) == 0 )
    {
        UtlString compareToToken;
        if ( loginPassToken.length() != MD5_DIGEST_LENGTH )
        {
            // we have to create the MD5 Digest
            UtlString compareToToken;
            UtlString textToEncode =
                userOrExtensionAtOptDomain + ":" +
                realmName + ":" +
                loginPassToken;
            NetMd5Codec::encode( textToEncode, compareToToken );
            if ( dbPassToken.compareTo ( compareToToken ) == 0 )
                result = TRUE;
        }
        else // potentially MD5 encoded password as length match
        {
            if ( loginPassToken.compareTo ( dbPassToken ) != 0 )
            {   // length matched but failed comparison
                // likely a paranoid user! Hash their 32 byte PW
                // we have to create the MD5 Digest
                UtlString textToEncode =
                    userOrExtensionAtOptDomain + ":" +
                    realmName + ":" +
                    loginPassToken;
                NetMd5Codec::encode( textToEncode, compareToToken );
                if ( dbPassToken.compareTo ( compareToToken ) == 0 )
                    result = TRUE;
            }
            else  //passwords match! no need to do anything
            {
                result = TRUE;
            }
        }
    }

    return result;
}
