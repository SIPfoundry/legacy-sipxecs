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
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/DeleteGreetingCGI.h"
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

DeleteGreetingCGI::DeleteGreetingCGI(const UtlBoolean& requestIsFromWebUI,
                                     const UtlString& mailbox,
                                     const UtlString& greetingType,
                                     const UtlBoolean& isActiveGreeting) :
    m_mailboxIdentity ( mailbox ),
    m_greetingType ( greetingType ),
    m_fromWeb( requestIsFromWebUI ),
    m_isActiveGreeting ( isActiveGreeting )
{}

DeleteGreetingCGI::~DeleteGreetingCGI()
{}

OsStatus
DeleteGreetingCGI::execute(UtlString* out)
{
    // Validate the mailbox id and extension.
    ValidateMailboxCGIHelper validateMailboxHelper ( m_mailboxIdentity );
    OsStatus result = validateMailboxHelper.execute( out );
    if( result == OS_SUCCESS )
    {
        validateMailboxHelper.getMailboxIdentity( m_mailboxIdentity );
            if( m_fromWeb )
                    result = handleWebRequest( out ) ;
            else
                    result = handleOpenVXIRequest( out ) ;
    }

    return result;
}

OsStatus
DeleteGreetingCGI::handleOpenVXIRequest(UtlString* out)
{
    // Currently, this CGI cannot be invoked from the OpenVXI.
        return OS_SUCCESS;
}

OsStatus
DeleteGreetingCGI::handleWebRequest(UtlString* out)
{
    UtlString redirectUrl, dynamicHtml ;

    // Retrieve the mediaserver host -- https://localhost:8091
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;
    if( result == OS_SUCCESS )
    {
        // Call the mailbox manager method for deleting the greeting
            result = pMailboxManager->deleteGreeting(   m_mailboxIdentity,
                                                    m_greetingType,
                                                    m_isActiveGreeting
                                                 );

        // Construct the redirect URL.
        redirectUrl +=  UtlString( CGI_URL ) +
                        "?action=getallgreetings&fromweb=yes&status=";

        // Add appropriate return status to the redirect URL
            if( result == OS_SUCCESS )
            redirectUrl += DELETE_GREETING_SUCCESS ;
            else
            redirectUrl += DELETE_GREETING_FAILED ;

        // Generate the HTML for redirecting to the 'redirectURL'
            dynamicHtml =   HTML_BEGIN \
                        REDIRECT_SCRIPT_BEGIN + redirectUrl + REDIRECT_SCRIPT_END \
                        HTML_END ;
    }
    else
    {
        dynamicHtml =   HTML_BEGIN \
                        PROTOCOL_MISMATCH \
                        HTML_END ;
    }

        // Write out the dynamic VXML script to be processed by OpenVXI
        if (out)
        {
                out->remove(0);
                out->append(dynamicHtml.data());
        }

    return OS_SUCCESS ;
}
