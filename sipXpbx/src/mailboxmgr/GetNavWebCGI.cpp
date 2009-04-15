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
#include "mailboxmgr/GetNavWebCGI.h"
#include "mailboxmgr/HTMLDefs.h"
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

GetNavWebCGI::GetNavWebCGI( const UtlString& mailboxIdentity ) :
    m_mailboxIdentity ( mailboxIdentity)
{}

GetNavWebCGI::~GetNavWebCGI()
{}

OsStatus
GetNavWebCGI::execute(UtlString* out)
{
    UtlString redirectUrl, dynamicHtml ;
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;

    if( result == OS_SUCCESS )
    {
            // Validate the mailbox id and extension.
        ValidateMailboxCGIHelper validateMailboxHelper ( m_mailboxIdentity );
        result = validateMailboxHelper.execute( out );
        if ( result == OS_SUCCESS )
            redirectUrl += NAV_URL ;
            else
                    redirectUrl += VOICEMAIL_NOT_ENABLED_NAV_URL;

            dynamicHtml  =      HTML_BEGIN \
                        REDIRECT_SCRIPT_BEGIN + redirectUrl + REDIRECT_SCRIPT_END \
                        HTML_END ;
    }
    else
    {
        dynamicHtml =   HTML_BEGIN \
                        PROTOCOL_MISMATCH \
                        HTML_END ;
    }

        if (out)
        {
                out->remove(0);
                out->append(dynamicHtml.data());
        }

        return OS_SUCCESS ;
}
