//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/SetActiveSystemPromptCGI.h"
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

SetActiveSystemPromptCGI::SetActiveSystemPromptCGI(     const UtlBoolean& requestIsFromWebUI,
                                                    const UtlString& promptType) :
    m_promptType ( promptType ),
    m_fromWeb( requestIsFromWebUI )
{}

SetActiveSystemPromptCGI::~SetActiveSystemPromptCGI()
{}

OsStatus
SetActiveSystemPromptCGI::execute(UtlString* out)
{
    OsStatus result;
        if( m_fromWeb )
                result = handleWebRequest( out ) ;
        else
                result = handleOpenVXIRequest( out ) ;

    return result;
}

OsStatus
SetActiveSystemPromptCGI::handleOpenVXIRequest(UtlString* out)
{
        // Instantiate the mailbox manager
    MailboxManager* pMailboxManager = MailboxManager::getInstance();

        // Call the method on Mailbox Manager to do the actual work.
        OsStatus result = pMailboxManager->setActiveSystemPrompt( m_promptType );

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
SetActiveSystemPromptCGI::handleWebRequest(UtlString* out)
{

    return OS_SUCCESS ;
}
