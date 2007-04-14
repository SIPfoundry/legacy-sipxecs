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
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ActiveGreetingHelper.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

ActiveGreetingHelper::ActiveGreetingHelper()
{}

ActiveGreetingHelper::~ActiveGreetingHelper()
{}

OsStatus
ActiveGreetingHelper::getActiveGreetingUrl( const UtlString& mailboxIdentity,
                                            UtlString& rGreetingUrl,
                                            const UtlBoolean& isFromWeb) const
{
    OsStatus result = OS_SUCCESS;

        // Instantiate the mailbox manager
    MailboxManager* pMailboxManager =
        MailboxManager::getInstance();

        // Call the method on Mailbox Manager to do the actual work.
    result = pMailboxManager->getActiveGreeting( mailboxIdentity, rGreetingUrl, isFromWeb );

    return result;
}


OsStatus
ActiveGreetingHelper::getActiveGreetingType( const UtlString& mailboxIdentity,
                                             UtlString& rGreetingType ) const
{
    OsStatus result = OS_SUCCESS;

        // Instantiate the mailbox manager
    MailboxManager* pMailboxManager =
        MailboxManager::getInstance();

        // Call the method on Mailbox Manager to do the actual work.
    result = pMailboxManager->getActiveGreetingType( mailboxIdentity, rGreetingType );

    return result;
}


OsStatus
ActiveGreetingHelper::getRecordedName(  const UtlString& mailboxIdentity,
                                        UtlString& rGreeting,
                                        const UtlBoolean& isFromWeb) const
{
    OsStatus result = OS_SUCCESS;

        // Instantiate the mailbox manager
    MailboxManager* pMailboxManager =
        MailboxManager::getInstance();

        // Call the method on Mailbox Manager to do the actual work.
    result = pMailboxManager->getRecordedName( mailboxIdentity, rGreeting, isFromWeb );

    return result;
}

OsStatus
ActiveGreetingHelper::getGreetingUrl(   const UtlString& mailboxIdentity,
                                        const UtlString& greetingType,
                                        UtlString& rGreetingUrl,
                                        const UtlBoolean& isFromWeb,
                                        const UtlBoolean& returnDefaultFileUrl) const
{
    OsStatus result = OS_SUCCESS;

        // Instantiate the mailbox manager
    MailboxManager* pMailboxManager =
        MailboxManager::getInstance();

        // Call the method on Mailbox Manager to do the actual work.
    result = pMailboxManager->getGreetingUrl(   mailboxIdentity,
                                                greetingType,
                                                rGreetingUrl,
                                                isFromWeb,
                                                returnDefaultFileUrl);

    return result;
}
