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
#include "mailboxmgr/RecycleDeletedMsgsCGI.h"
#include "mailboxmgr/HTMLDefs.h"
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

RecycleDeletedMsgsCGI::RecycleDeletedMsgsCGI(const UtlBoolean& requestIsFromWebUI,
                                             const UtlString& mailbox,
                                             const UtlString& messageIds,
                                             const UtlString& nextblockhandle) :
    m_mailboxIdentity ( mailbox ),
    m_fromWeb( requestIsFromWebUI ),
    m_messageids ( messageIds ),
    m_nextBlockHandle (nextblockhandle)
{}

RecycleDeletedMsgsCGI::~RecycleDeletedMsgsCGI()
{}

OsStatus
RecycleDeletedMsgsCGI::execute(UtlString* out)
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
RecycleDeletedMsgsCGI::handleWebRequest( UtlString* out )
{
    UtlString redirectUrl, dynamicHtml;
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;
    if( result == OS_SUCCESS )
    {
        // URL of the result webpage
        redirectUrl +=  UtlString( CGI_URL ) +
                        "?action=playmsg&fromweb=yes&from=gateway" +
                        "&nextblockhandle=" + UtlString( m_nextBlockHandle ) +
                        "&category=deleted";
        if( m_messageids.length() > 0 )
        {
            // Instantiate the mailbox manager
                result = pMailboxManager->recycleDeletedMessages( m_mailboxIdentity, m_messageids );

            // Add status to indicate the status of purge.
                if( result == OS_SUCCESS )
            {
                redirectUrl +=  UtlString( "&status=" ) +
                                RECYCLE_DELETED_MSG_SUCCESSFUL ;
            }
            else
            {
                        redirectUrl +=  UtlString( "&status=" ) +
                                RECYCLE_DELETED_MESSAGES_FAILED ;
            }
        }
        else
        {
            redirectUrl +=  UtlString( "&status=" ) +
                            RECYCLE_DELETED_MSG_NOT_SELECTED ;
        }

        // Script for redirecting to the webpage displaying folder contents
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
        return OS_SUCCESS;
}

OsStatus
RecycleDeletedMsgsCGI::handleOpenVXIRequest( UtlString* out )
{
    // Instantiate the mailbox manager
    MailboxManager* pMailboxManager = MailboxManager::getInstance();

        OsStatus result = pMailboxManager->recycleDeletedMessages( m_mailboxIdentity, m_messageids );

        UtlString dynamicVxml = getVXMLHeader();
        if( result == OS_SUCCESS )
        {
                dynamicVxml += VXML_SUCCESS_SNIPPET;
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
