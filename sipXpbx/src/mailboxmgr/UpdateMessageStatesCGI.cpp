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
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/UpdateMessageStatesCGI.h"
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

UpdateMessageStatesCGI::UpdateMessageStatesCGI(
    const UtlBoolean& requestIsFromWebUI,
    const UtlBoolean& linkInEmail,
    const UtlString& mailboxIdentity,
    const UtlString& category,
    const UtlString& messageIds) :
    m_mailboxIdentity ( mailboxIdentity ),
    m_category ( category ),
    m_messageIds ( messageIds ),
    m_fromWeb( requestIsFromWebUI ),
    m_linkInEmail ( linkInEmail )
{}

UpdateMessageStatesCGI::~UpdateMessageStatesCGI()
{}

OsStatus
UpdateMessageStatesCGI::execute(UtlString* out)
{
        // Validate the mailbox id and extension.
    ValidateMailboxCGIHelper validateMailboxHelper ( m_mailboxIdentity );
    OsStatus result = validateMailboxHelper.execute( out );
    if( result == OS_SUCCESS )
    {
        validateMailboxHelper.getMailboxIdentity( m_mailboxIdentity );
        if( m_fromWeb )
        {
            if( m_linkInEmail )
                result = handleEmailRequest( out );
            else
                    result = handleWebRequest( out ) ;
        }
            else
                    result = handleOpenVXIRequest( out ) ;
    }
    return result;
}

OsStatus
UpdateMessageStatesCGI::handleWebRequest( UtlString* out )
{
    // Instantiate the mailbox manager
    UtlString redirectUrl, dynamicHtml ;
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;

    if( result == OS_SUCCESS )
    {
       // We have revised the links that invoke this CGI pathway
       // so that the messageId parameter is now just the eight-digit
       // message ID.
       UtlString messageId = m_messageIds;

       result = pMailboxManager->updateMessageStates(
          m_mailboxIdentity, m_category, messageId);

       // URL of the message WAV file
       Url url ( m_mailboxIdentity );
       UtlString userId;
       url.getUserId( userId );

       // Null HTML file.
       dynamicHtml  =   HTML_BEGIN \
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

    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "UpdateMessageStatesCGI::handleWebRequest: out = '%s'",
                  out->data());
    return OS_SUCCESS;
}


OsStatus
UpdateMessageStatesCGI::handleEmailRequest( UtlString* out )
{
   // Instantiate the mailbox manager
   UtlString redirectUrl, dynamicHtml ;
   MailboxManager* pMailboxManager = MailboxManager::getInstance();
   OsStatus result = pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;

   if( result == OS_SUCCESS )
   {
      // Strip of the "-dd.wav" from messageIds.
      UtlString messageId = m_messageIds( 0, m_messageIds.length()-7);

      result = pMailboxManager->updateMessageStates(
         m_mailboxIdentity, m_category, messageId);

      if (result == OS_SUCCESS)
      {
         // URL of the message WAV file
         Url url ( m_mailboxIdentity );
         UtlString userId;
         url.getUserId( userId );

         redirectUrl +=  UtlString( URL_SEPARATOR ) + MEDIASERVER_ROOT_ALIAS +
            URL_SEPARATOR + MAILBOX_DIR +
            URL_SEPARATOR + userId +
            URL_SEPARATOR + m_category +
            URL_SEPARATOR + m_messageIds ;

         // Script for playing the WAV file
         dynamicHtml  = HTML_BEGIN \
            EMBED_MEDIAPLAYER_PLUGIN \
            "<SCRIPT language=\"JavaScript\">\n" \
            "<!-- \n"
            "playMsgJs('" + redirectUrl + "'); \n" \
            "// -->\n" \
            "</SCRIPT>\n" \
            "<b class=\"statustext\">Message is now playing. Please make sure your PC speaker is turned on.</b>" \
            HTML_END ;
      }
      else if (result == OS_NOT_FOUND)
      {
         dynamicHtml =   HTML_BEGIN \
            "Message is not found in " + m_category + ", may have been saved or deleted." \
            HTML_END ;
      }
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
UpdateMessageStatesCGI::handleOpenVXIRequest( UtlString* out )
{
    // Instantiate the mailbox manager
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
        OsStatus result = pMailboxManager->updateMessageStates(m_mailboxIdentity, m_category, m_messageIds);

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
