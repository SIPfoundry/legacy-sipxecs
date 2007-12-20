//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlRegex.h"
#include "net/Url.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */
ValidateMailboxCGIHelper::ValidateMailboxCGIHelper( const UtlString& identityOrExtension ) :
    m_identityOrExtension (identityOrExtension),
    m_mailboxIdentity (""),
    m_extension (""),
    m_isValidated (false)
{
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "ValidateMailboxCGIHelper::ValidateMailboxCGIHelper('%s') called",
                 m_identityOrExtension.data());
}

ValidateMailboxCGIHelper::~ValidateMailboxCGIHelper()
{}

OsStatus
ValidateMailboxCGIHelper::execute( UtlString* out )
{
   OsStatus result = validate() ;
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "ValidateMailboxCGIHelper::execute: returns %d",
                 result);
   return result ;
}

OsStatus
ValidateMailboxCGIHelper::validate( const MailboxPermissions requiredPermissions )
{
    OsStatus result = OS_SUCCESS;
    // Check to see whether we have already validated the mailbox
    if (!m_isValidated)
    {
        // this local member call is on the IMDB utility class and not the mailbox (@JC TODO)
        result = validateIdentityAndGetExtension (
            m_mailboxIdentity,
            m_extension,
            requiredPermissions );
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "ValidateMailboxCGIHelper::validate: requiredPermissions = %d, returns m_mailboxIdentity = '%s', m_extension = '%s', result = %d",
                      requiredPermissions, m_mailboxIdentity.data(),
                      m_extension.data(), result);
        m_isValidated = true;
    }
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "ValidateMailboxCGIHelper::validate: returns %d",
                  result);
    return result;
}

OsStatus
ValidateMailboxCGIHelper::validateIdentityAndGetExtension (
    UtlString& rMailboxIdentity,
    UtlString& rExtension,
    const MailboxPermissions requiredPermissions)
{
    OsStatus result = OS_SUCCESS;
    // look up the identity and extension from the IMDB
    // @JC TODO during refactoring this will be a special IMDB
    // utility class and not the mailbox manager
    // Check to see whether we have already validated the mailbox
    if ( !m_isValidated )
    {
        result = MailboxManager::getInstance()->
                    validateMailbox (
                        m_identityOrExtension,
                        TRUE,      // resolve extension
                        requiredPermissions,
                        m_mailboxIdentity,
                        m_extension);
        // pass internal results back
        rMailboxIdentity = m_mailboxIdentity;
        rExtension = m_extension;
        m_isValidated = true;
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "ValidateMailboxCGIHelper::validateIdentityAndGetExtension: m_identityOrExtension = '%s', requiredPermissions = %d, returns m_mailboxIdentity = '%s', m_extension = '%s', result = %d",
                      m_identityOrExtension.data(), requiredPermissions,
                      m_mailboxIdentity.data(), m_extension.data(), result);
    }
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "ValidateMailboxCGIHelper::validateIdentityAndGetExtension: requiredPermissions = %d, result = %d",
                  requiredPermissions, result);
    return result;
}

void
ValidateMailboxCGIHelper::getExtension( UtlString& extension ) const
{
    extension = m_extension;
}

void
ValidateMailboxCGIHelper::getMailboxIdentity ( UtlString& mailboxIdentity ) const
{
    mailboxIdentity = m_mailboxIdentity;
}

UtlBoolean
ValidateMailboxCGIHelper::isNumeric( const UtlString& mailboxIdentity ) const
{
    UtlString userid ;
    getUserId( mailboxIdentity, userid ) ;

    static RegEx sAllDigits("^\\d+$");

    return sAllDigits.Search(userid.data());
}

OsStatus
ValidateMailboxCGIHelper::getUserId( const UtlString& mailboxIdentity,
                                     UtlString& rUserId ) const
{
    // Strip off the domain part from the identity
    if( mailboxIdentity.first( '@' ) == UTL_NOT_FOUND )
    {
        rUserId = mailboxIdentity ;
    }
    else
    {
        // Construct the Url object and get the userid.
        Url url( mailboxIdentity );
        url.getUserId( rUserId ) ;
    }

    return OS_SUCCESS ;
}
