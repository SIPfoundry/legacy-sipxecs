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
#include "net/Url.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/TransferToExtnCGI.h"
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

TransferToExtnCGI::TransferToExtnCGI( const UtlString& extension ) :
    m_extension ( extension )
{}

TransferToExtnCGI::~TransferToExtnCGI()
{}

OsStatus
TransferToExtnCGI::execute(UtlString* out)
{
    OsStatus result = OS_SUCCESS;

    // Get fully qualified SIP URL for the extension
    Url extensionUrl;
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    pMailboxManager->getWellFormedURL( m_extension, extensionUrl );

    // Validate the mailbox id and extension.
    // If valid, lazy create the physical folders for the mailbox if necessary.
    ValidateMailboxCGIHelper validateMailboxHelper ( extensionUrl.toString() );
    result = validateMailboxHelper.validate( MB_REQUIRE_NONE );
    UtlString dynamicVxml;
    if ( result == OS_SUCCESS )
    {
/*
        UtlString transferUrlString;
        validateMailboxHelper.getMailboxIdentity( transferUrlString );
        if (transferUrlString.index("sip:") == UTL_NOT_FOUND)
        {
            transferUrlString = "sip:" + transferUrlString;
        }
*/
        UtlString ivrPromptUrl;
        MailboxManager::getInstance()->getIvrPromptURL( ivrPromptUrl );

        // Contains the dynamically generated VXML script.
            dynamicVxml =  getVXMLHeader() +
                        "<form> \n" \
                            "<transfer dest=\"" + extensionUrl.toString() /* transferUrlString */ + "\" /> \n" \
                        "</form> \n" \
                        VXML_END;
            // Write out the dynamic VXML script to be processed by OpenVXI
    } else
    {
        dynamicVxml =  getVXMLHeader() + 
                       VXML_INVALID_EXTN_SNIPPET\
                       VXML_END;
    }

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
