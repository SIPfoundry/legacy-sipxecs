//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/SetActiveGreetingCGI.h"
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

SetActiveGreetingCGI::SetActiveGreetingCGI( const UtlBoolean& requestIsFromWebUI,
                                            const UtlString& mailbox,
                                            const UtlString& greetingType) :
    m_mailboxIdentity ( mailbox ),
    m_greetingType ( greetingType ),
    m_fromWeb( requestIsFromWebUI )
{}

SetActiveGreetingCGI::~SetActiveGreetingCGI()
{}

OsStatus
SetActiveGreetingCGI::execute(UtlString* out)
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
SetActiveGreetingCGI::handleOpenVXIRequest(UtlString* out)
{
        // Instantiate the mailbox manager
    MailboxManager* pMailboxManager = MailboxManager::getInstance();

        // Call the method on Mailbox Manager to do the actual work.
        OsStatus result = pMailboxManager->setActiveGreeting(m_mailboxIdentity, m_greetingType );

        UtlString dynamicVxml = getVXMLHeader();

        if( result == OS_SUCCESS )
        {
                dynamicVxml += VXML_SUCCESS_SNIPPET;
        }
        else if( result == OS_FILE_NOT_FOUND )
    {
        dynamicVxml +=  "<form> <block>\n" \
                                "<var name=\"result\" expr=\"'filenotfound'\"/>\n" \
                                "<return namelist=\"result\"/>\n"
                        "</block> </form>\n" ;
    }
    else
        {
                dynamicVxml += VXML_FAILURE_SNIPPET;
        }

        dynamicVxml += VXML_END;

        // Write out the dynamic VXML script to be processed by OpenVXI
        if (out)
        {
        out->remove(0);
        UtlString responseHeaders;
        MailboxManager::getResponseHeaders(dynamicVxml.length(), responseHeaders);

        out->append(responseHeaders.data());
        out->append(dynamicVxml.data());
        }
        return OS_SUCCESS;
}

OsStatus
SetActiveGreetingCGI::handleWebRequest(UtlString* out)
{
    UtlString redirectUrl, dynamicHtml ;
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;
    if( result == OS_SUCCESS )
    {
        redirectUrl +=  CGI_URL \
                        "?action=getallgreetings&fromweb=yes&status=";

        // Instantiate the mailbox manager
            result = pMailboxManager->setActiveGreeting(m_mailboxIdentity, m_greetingType );
            if( result == OS_SUCCESS )
            redirectUrl += SET_ACTIVE_GREETING_SUCCESS ;
            else
            redirectUrl += SET_ACTIVE_GREETING_FAILED ;

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
