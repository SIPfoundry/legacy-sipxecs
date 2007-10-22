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
#include "mailboxmgr/LoginCGI.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"
#include "sipdb/PermissionDB.h"
#include "net/Url.h"



// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

LoginCGI::LoginCGI( const UtlBoolean& requestIsFromWebUI,
                                        const UtlString& userid,
                                        const UtlString& password ) :
        m_fromWebUI( requestIsFromWebUI ),
        m_userid ( userid ),
        m_password ( password ),
    m_mailboxIdentity (""),
    m_extension ("")
{}

LoginCGI::~LoginCGI()
{}

void
LoginCGI::getMailboxIdentity( UtlString& mailboxIdentity ) const
{
    mailboxIdentity = m_mailboxIdentity;
}

void
LoginCGI::getExtension( UtlString& extension ) const
{
    extension = m_extension;
}

OsStatus
LoginCGI::execute(UtlString* out)
{
        OsStatus result ;
        if( m_fromWebUI )
                result = handleWebRequest( out ) ;
        else
                result = handleOpenVXIRequest( out ) ;


    return result;
}

OsStatus
LoginCGI::handleWebRequest( UtlString* out )
{
    UtlString redirectUrl, dynamicHtml ;
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;
    if( result == OS_SUCCESS )
    {
            // Validate the mailbox id and extension.
        ValidateMailboxCGIHelper validateMailboxHelper ( m_userid );
        result = validateMailboxHelper.execute( out );
            if ( result == OS_SUCCESS )
                    redirectUrl += CGI_URL + UtlString( "?action=retrieve&fromweb=yes" ) ;
            else
                    redirectUrl += VOICEMAIL_NOT_ENABLED_URL;

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

OsStatus
LoginCGI::handleOpenVXIRequest( UtlString* out )
{
    OsStatus result = OS_FAILED;

    // Instantiate the mailbox manager
    MailboxManager* pMailboxManager =
        MailboxManager::getInstance();

        UtlString dynamicVxml = getVXMLHeader();

        // Update the m_mailbox if valid
    result = pMailboxManager->doLogin(
                m_userid,           // this is an extension
                m_password,         // this is actually a PIN
                m_mailboxIdentity,  // filld in on return
                m_extension );      // filled in on return

        if ( result == OS_SUCCESS )
        {
                // Stores the number of unheard messages in inbox.
                UtlString       rUnheardCount;

                // Stores the total number of mesages in inbox
                UtlString       rTotalCount;

                // Stores the total number of messages in saved folder.
                UtlString       rSavedCount;

                // Stores the total number of unheard messages in saved folder.
                // Note: This is not used.
                UtlString       rSavedUnheardCount;

                // Call the method on Mailbox Manager to get the mailbox status
                result = pMailboxManager->getMailboxStatus( m_mailboxIdentity,
                                                           "inbox",
                                                           rUnheardCount,
                                                           rTotalCount );

                result = pMailboxManager->getMailboxStatus( m_mailboxIdentity,
                                                           "saved",
                                                           rSavedUnheardCount,
                                                           rSavedCount );

        // Check if this user has permission to record system prompts.
        Url mailboxUrl ( m_mailboxIdentity );
        UtlBoolean bHasRecordSystemPromptsPermission = PermissionDB::getInstance()->
                        hasPermission( mailboxUrl, RECORD_SYSTEM_PROMPTS_PERMISSION );

        UtlString strRecordEnabled = "no" ;
        if( bHasRecordSystemPromptsPermission )
            strRecordEnabled = "yes" ;

                dynamicVxml += "<form>\n<block>\n";
                dynamicVxml += "<var name=\"result\" expr=\"'success'\"/>\n";

                // Construct the VXML scriptlet.
                if( result == OS_SUCCESS )
                {
                        dynamicVxml +=  "<var name=\"unheard\" expr=\"" + rUnheardCount + "\"/>\n"\
                                                        "<var name=\"total\" expr=\"" + rTotalCount + "\"/>\n"\
                                                        "<var name=\"saved\" expr=\"" + rSavedCount + "\"/>\n"\
                                                        "<var name=\"mailboxid\" expr=\"'" + m_mailboxIdentity + "'\"/>\n" \
                                                        "<var name=\"extension\" expr=\"'" + m_extension + "'\"/>\n" \
                            "<var name=\"isRecordSystemPromptsEnabled\" expr=\"'" + strRecordEnabled + "'\"/>\n" ;

        } else
                {
                        dynamicVxml +=  "<var name=\"unheard\" expr=\"0\"/>"\
                                                        "<var name=\"total\" expr=\"0\"/>"\
                                                        "<var name=\"saved\" expr=\"0\"/>"\
                                                        "<var name=\"mailboxid\" expr=\"'" + m_mailboxIdentity + "'\"/>\n" \
                                                        "<var name=\"extension\" expr=\"'" + m_extension + "'\"/>\n" \
                            "<var name=\"isRecordSystemPromptsEnabled\" expr=\"'" + strRecordEnabled + "'\"/>\n" ;
                }
                dynamicVxml +=  "<return namelist=\"result unheard total saved mailboxid extension isRecordSystemPromptsEnabled\"/>\n"\
                                                "</block>\n</form>\n";
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

        return OS_SUCCESS ;

}
