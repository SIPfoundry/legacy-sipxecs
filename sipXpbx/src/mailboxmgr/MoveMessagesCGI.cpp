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
#include "mailboxmgr/MoveMessagesCGI.h"
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

MoveMessagesCGI::MoveMessagesCGI(   const UtlBoolean& requestIsFromWebUI,
                                    const UtlString& mailbox,
                                    const UtlString& fromFolder,
                                    const UtlString& toFolder,
                                    const UtlString& messageIds,
                                    const UtlString& maintainstatus,
                                    const UtlString& nextblockhandle) :
m_mailboxIdentity ( mailbox ),
m_fromFolder ( fromFolder ),
m_toFolder ( toFolder ),
m_messageIds ( messageIds ),
m_maintainstatus ( maintainstatus ),
m_fromWeb( requestIsFromWebUI ),
m_nextBlockHandle (nextblockhandle)
{
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MoveMessagesCGI::MoveMessagesCGI: requestIsFromWebUI = %d, mailbox = '%s', fromFolder = '%s', toFolder = '%s', messageIds = '%s', maintainstatus = '%s', nextblockhandle = '%s'",
                 requestIsFromWebUI, mailbox.data(), fromFolder.data(),
                 toFolder.data(), messageIds.data(), maintainstatus.data(),
                 nextblockhandle.data());
}

MoveMessagesCGI::~MoveMessagesCGI()
{}

OsStatus
MoveMessagesCGI::execute(UtlString* out)
{
    // Validate the mailbox id
    ValidateMailboxCGIHelper validateMailboxHelper ( m_mailboxIdentity );
    OsStatus result = validateMailboxHelper.execute( out );
    if ( result == OS_SUCCESS )
    {
        // Get the fully qualified mailbox identity
        validateMailboxHelper.getMailboxIdentity(m_mailboxIdentity);

        if ( m_fromWeb )
            result = handleWebRequest( out ) ;
        else
            result = handleOpenVXIRequest( out ) ;
    }

    return result;
}

OsStatus
MoveMessagesCGI::handleWebRequest( UtlString* out )
{
    // URL of the result webpage
    UtlString redirectUrl, dynamicHtml ;
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;

    if ( result == OS_SUCCESS )
    {
        redirectUrl +=  UtlString( CGI_URL ) +
                        "?action=playmsg&fromweb=yes&from=gateway" +
                        "&nextblockhandle=" + UtlString( m_nextBlockHandle ) +
                        "&category=" + m_fromFolder;

        if ( m_toFolder.compareTo( "1", UtlString::ignoreCase) == 0 )
        {
            if ( m_messageIds.length() <= 0 )
                redirectUrl +=  UtlString( "&status=" ) +
                                MOVE_MSG_AND_FOLDER_NOT_SELECTED ;
            else
                redirectUrl +=  UtlString( "&status=" ) +
                                MOVE_FOLDER_NOT_SELECTED ;
        }
        else if( m_messageIds.length() > 0 )
        {
            // Call MailboxManager to move the messages
            MailboxManager* pMailboxManager = MailboxManager::getInstance();
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MoveMessagesCGI::handleWebRequest: call MailboxManager::moveMessages( m_mailboxIdentity = '%s', m_fromFolder = '%s', m_toFolder = '%s', m_messageIds = '%s', m_maintainstatus = '%s'",
                          m_mailboxIdentity.data(), m_fromFolder.data(),
                          m_toFolder.data(), m_messageIds.data(),
                          m_maintainstatus.data());
            result = pMailboxManager->moveMessages( m_mailboxIdentity, m_fromFolder, m_toFolder, m_messageIds, m_maintainstatus );
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MoveMessagesCGI::handleWebRequest: MailboxManager::moveMessages() returns %d",
                          result);

            // Add status to indicate that the move operation failed
            if ( result == OS_SUCCESS )
            {
                if ( m_toFolder == "deleted" )
                    redirectUrl +=  UtlString( "&status=" ) +
                                    MSG_DELETED_SUCCESSFULLY ;
                else
                    redirectUrl +=  UtlString( "&status=" ) +
                                    MSG_MOVED_SUCCESSFULLY ;
            } else
            {
                if ( m_toFolder == "deleted" )
                    redirectUrl +=  UtlString( "&status=" ) +
                                    DELETE_MSG_FAILED ;
                else
                    redirectUrl +=  UtlString( "&status=" ) +
                                    MOVE_MSG_FAILED ;
            }
        }
        else
        {
            if ( m_toFolder == "deleted" )
                redirectUrl +=  UtlString( "&status=" ) +
                                DELETE_MSG_NOT_SELECTED ;
            else
                redirectUrl +=  UtlString( "&status=" ) +
                                MOVE_MSG_NOT_SELECTED ;
        }

        // Script for redirecting to the webpage displaying folder contents
        dynamicHtml  =  HTML_BEGIN \
                        REDIRECT_SCRIPT_BEGIN + redirectUrl + REDIRECT_SCRIPT_END \
                        HTML_END ;
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "MoveMessagesCGI::handleWebRequest: redirectUrl = '%s'",
                      redirectUrl.data());
    }
    else
    {
        dynamicHtml =   HTML_BEGIN \
                        PROTOCOL_MISMATCH \
                        HTML_END ;
    }

    if ( out )
    {
        out->remove(0);
        out->append(dynamicHtml.data());
    }

    return OS_SUCCESS;
}

OsStatus
MoveMessagesCGI::handleOpenVXIRequest( UtlString* out )
{
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = pMailboxManager->moveMessages( m_mailboxIdentity, m_fromFolder, m_toFolder, m_messageIds, m_maintainstatus );

    UtlString dynamicVxml = getVXMLHeader();

    if ( result == OS_SUCCESS )
    {
        dynamicVxml += VXML_SUCCESS_SNIPPET;
        }
        else
    {
        dynamicVxml += VXML_FAILURE_SNIPPET;
    }

    dynamicVxml += VXML_END;

    // Write out the dynamic VXML script to be processed by OpenVXI
    if ( out )
    {
        out->remove(0);
        UtlString responseHeaders;
        MailboxManager::getResponseHeaders(dynamicVxml.length(), responseHeaders);

        out->append(responseHeaders.data());
        out->append(dynamicVxml.data());
    }

    return OS_SUCCESS;
}
