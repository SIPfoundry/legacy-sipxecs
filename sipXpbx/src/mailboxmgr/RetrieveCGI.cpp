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
#include "net/HttpMessage.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/RetrieveCGI.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"

// DEFINES
#define UNKNOWN_EXTENSION   "-1"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

RetrieveCGI::RetrieveCGI ( 
   const UtlBoolean& requestIsFromWebUI,
   const UtlString& from ) :
   m_from ( from ),
   m_fromWebUI( requestIsFromWebUI )
{}

RetrieveCGI::~RetrieveCGI()
{}

OsStatus
RetrieveCGI::execute(UtlString* out)
{
   OsStatus result;
   if( m_fromWebUI )
      result = handleWebRequest( out );
   else
      result = handleOpenVXIRequest( out );

   return result;
}

OsStatus
RetrieveCGI::handleWebRequest( UtlString* out )
{
   UtlString outputString;

   // Validate the mailbox id and extension.
   // If valid, lazy create the physical folders for the mailbox if necessary.
   ValidateMailboxCGIHelper validateMailboxHelper ( m_from );
   OsStatus result = validateMailboxHelper.execute( out );

   // Get the time in minutes after which the page should refresh automatically.
   UtlString refreshInterval;

   if ( result == OS_SUCCESS ) 
   {
      // Get the fully qualified mailbox id.
      UtlString fromIdentity;
      validateMailboxHelper.getMailboxIdentity( fromIdentity );

      // Stores the number of unheard & total count of messages in inbox.
      UtlString rUnheardCount, rTotalCount;

      // Instantiate the mailbox manager
      MailboxManager* pMailboxManager = MailboxManager::getInstance();

      // Call the method on Mailbox Manager to get the mailbox status
      result = pMailboxManager->getMailboxStatus( fromIdentity, 
                                                  "inbox",
                                                  rUnheardCount, 
                                                  rTotalCount );
      if( rUnheardCount == "0" )
         outputString = "You have no new messages in your inbox";
      else if( rUnheardCount == "1" )
         outputString = "You have " + rUnheardCount + " new message in your inbox";
      else
         outputString = "You have " + rUnheardCount + " new messages in your inbox";

      pMailboxManager->getPageRefreshInterval( refreshInterval );

   } else
   {
      outputString = "Voicemail is not enabled for this user";
   }

   
   UtlString dynamicHtml =  NO_HTML_CACHE_HEADER \
      "<HTML>\n" \
      "<HEAD>\n" \
      "<link rel=\"stylesheet\" href=\"/style/voicemail.css\" type=\"text/css\">\n" \
      "<script language=\"JavaScript\"> \n" \
      "var countDownInterval=" + refreshInterval + "*60; \n" \
      "var countDownTime=countDownInterval+1; \n" \
      "function countDown(){ \n" \
      "countDownTime--; \n" \
      "if (countDownTime <=0){ \n" \
      "countDownTime=countDownInterval; \n" \
      "clearTimeout(counter); \n" \
      "window.location.reload(); \n" \
      "return; \n" \
      "} \n" \
      "counter=setTimeout(\"countDown()\", 1000); \n" \
      "} \n" \
      "if (document.all||document.getElementById) \n" \
      "countDown(); \n" \
      "else \n" \
      "window.onload=countDown; \n" \
      "</script> \n" \
      "</HEAD>\n" \
      "<BODY class=\"bglight\">\n" \
      "<table width=\"560\" border=\"0\"> \n" \
      "<tr> \n" \
      "<td width=\"5\"><img src=\"/images/spacer.gif\" width=\"5\"></td> \n" \
      "<td width=\"100%\">\n" \
      "<table width=\"100%\" border=\"0\">\n" \
      "<tr>\n" \
      "<td align=\"left\"><b class=\"formtext\">Voicemail</b></td>\n" \
      "</tr>\n" \
      "<tr>\n" \
      "<td align=\"left\">" + outputString + "</td>\n" \
      "</tr>\n" \
      "</table>\n" \
      "</td>\n" \
      "</tr>\n" \
      "</table>\n" \
      "</BODY>\n" \
      "</HTML>";

   if (out) 
   {
      out->remove(0);
      out->append(dynamicHtml.data());
   }
   return OS_SUCCESS;
}

OsStatus
RetrieveCGI::handleOpenVXIRequest( UtlString* out )
{
   // contains the output of this CGI - dynamically generated vxml
   UtlString dynamicVxml = getVXMLHeader();
   dynamicVxml += "<form>";

   // Get mediaserver base URL
   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   UtlString ivrPromptUrl;
   pMailboxManager->getIvrPromptURL( ivrPromptUrl );

   UtlString mediaserverUrl;
   pMailboxManager->getMediaserverURL( mediaserverUrl );

   UtlString secureMediaserverUrl;
   pMailboxManager->getMediaserverSecureURL( secureMediaserverUrl );


   // Check if the call or login came from known mailbox user
   ValidateMailboxCGIHelper validateMailboxHelper( m_from );
   OsStatus result = validateMailboxHelper.execute( out );
   UtlString fromIdentity;
   if( result == OS_SUCCESS )
      validateMailboxHelper.getMailboxIdentity( fromIdentity );
   else
      fromIdentity = UNKNOWN_EXTENSION;

   // ensure we loose no information from the m_from field
   UtlString vxmlFriendlyFrom = m_from;
   // HttpMessage::escape( escapedFrom );
   MailboxManager::convertUrlStringToXML(vxmlFriendlyFrom);
   // Construct the VXML script for invoking login script.
   dynamicVxml +=  
      "<subdialog name=\"send_msg\" src=\"" + mediaserverUrl + "/vm_vxml/login.vxml\">\n" \
      "<param name=\"extn\" value=\"" + fromIdentity + "\"/>\n" \
      "<param name=\"from\" value=\"" + vxmlFriendlyFrom + "\"/>\n" \
      "<param name=\"mediaserverurl\" value=\"" + ivrPromptUrl + "\"/>\n" \
      "<param name=\"securemediaserverurl\" value=\"" + secureMediaserverUrl + "\"/>\n" \
      "</subdialog>";

   dynamicVxml += "</form>";
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


