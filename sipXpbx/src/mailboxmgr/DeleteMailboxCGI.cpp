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
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"
#include "mailboxmgr/DeleteMailboxCGI.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

DeleteMailboxCGI::DeleteMailboxCGI ( const UtlString& mailboxIdentity ) :
   m_mailboxIdentity ( mailboxIdentity )
{}

DeleteMailboxCGI::~DeleteMailboxCGI()
{}

OsStatus
DeleteMailboxCGI::execute(UtlString* out)
{
    ValidateMailboxCGIHelper validateMailboxHelper ( m_mailboxIdentity );

    UtlString validatedMailboxIdentity, extension;
    OsStatus result = validateMailboxHelper.
                          validateIdentityAndGetExtension(
                              validatedMailboxIdentity,
                              extension);
    // Construct a success result script.
    UtlString dynamicVxml (VXML_BODY_BEGIN);
    if ( result == OS_SUCCESS )
    {
        // Mailbox exists so delete it
        result = MailboxManager::getInstance()->
            deleteMailbox ( validatedMailboxIdentity );

        if ( result == OS_SUCCESS )
        {
            dynamicVxml += VXML_SUCCESS_SNIPPET;
        } else
        {
            dynamicVxml += VXML_FAILURE_SNIPPET;
        }
    } else
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


    return result;
}
