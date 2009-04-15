//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHashMap.h"
#include "net/Url.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/EditMessageWebCGI.h"
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

EditMessageWebCGI::EditMessageWebCGI(
    const UtlString& mailboxIdentity,
    const UtlString& folderName,
    const UtlString& messageId,
    const UtlString& subject,
    const UtlString& nextblockhandle,
    const UtlBoolean& formSubmitted) :
    m_mailboxIdentity ( mailboxIdentity),
    m_folderName ( folderName ),
    m_messageId ( messageId ),
    m_msgSubject ( subject ),
    m_formSubmitted ( formSubmitted ),
    m_nextBlockHandle ( nextblockhandle )
{}

EditMessageWebCGI::~EditMessageWebCGI()
{}

OsStatus
EditMessageWebCGI::execute(UtlString* out)
{
    UtlString redirectUrl, dynamicHtml ;

    // Retrieve the mediaserver base URL - https://localhost:8091
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;
    if( result == OS_SUCCESS )
    {
        // Construct the dynamic HTML.
        // 'subject' field is the first editable field in the form.
        // Provide focus to it on load.
        dynamicHtml =   HTML_BEGIN ;

            // Validate the mailbox id and extension.
        ValidateMailboxCGIHelper validateMailboxHelper ( m_mailboxIdentity );
        result = validateMailboxHelper.execute( out );
        if ( result == OS_SUCCESS )
        {
            // retrieve the fully qualified mailbox id.
            validateMailboxHelper.getMailboxIdentity( m_mailboxIdentity );

            redirectUrl += CGI_URL ;

            UtlString errorMsg ;
            bool redirected = false ;

            if( m_formSubmitted )
            {
                // User wants to submit the changes they made to the message.

                // Validate the subject
                if( ( m_msgSubject.compareTo("not set") == 0) ||
                    ( m_msgSubject.length() == 0) )
                {
                    errorMsg = "Please enter a valid subject" ;
                } else
                {
                    // Go ahead and update the subject in the XML file.
                    redirected = true ;
                    MailboxManager* pMailboxManager = MailboxManager::getInstance();
                        pMailboxManager->editMessage(
                        m_mailboxIdentity,
                        m_folderName,
                        m_messageId,
                        m_msgSubject ) ;

                    redirectUrl += "?action=playmsg&fromweb=yes&from=gateway" \
                                        "&nextblockhandle=" + UtlString( m_nextBlockHandle ) +
                                        "&category=" + UtlString( m_folderName ) +
                                        "&status=" + EDIT_MSG_SUCCESSFUL;
                    dynamicHtml += REDIRECT_SCRIPT_BEGIN +
                                   redirectUrl +
                                   REDIRECT_SCRIPT_END ;
                }
            }

            if( !redirected )
            {
                // Retrieve info about the message and construct the form for display.
                UtlHashMap* msgHashDictionary = new UtlHashMap();
                MailboxManager* pMailboxManager = MailboxManager::getInstance();
                    pMailboxManager->getMessageInfo(
                    m_mailboxIdentity,
                    m_folderName,
                    m_messageId,
                    msgHashDictionary ) ;

                UtlString* collectableKey =
                    new UtlString("from");

                        UtlString* value = (UtlString*) msgHashDictionary->findValue( collectableKey );

                        UtlString from = value->data();
                Url fromUrl = from.data();;
                fromUrl.getDisplayName(from);
                UtlString fromIdentity;
                fromUrl.getIdentity(fromIdentity);

                from += "&lt;sip:" + fromIdentity + "&gt;";

                delete collectableKey;
                delete value;

                collectableKey = new UtlString("timestamp") ;
                        value = (UtlString*) msgHashDictionary->findValue( collectableKey );
                        UtlString timestamp = value->data();
                delete collectableKey;
                delete value ;

                collectableKey = new UtlString("durationsecs") ;
                        value = (UtlString*) msgHashDictionary->findValue( collectableKey );
                        UtlString duration = value->data();
                delete collectableKey;
                delete value ;

                UtlString subject ;
                if( errorMsg.isNull() )
                {
                    collectableKey = new UtlString("subject") ;
                            value = (UtlString*) msgHashDictionary->findValue( collectableKey );
                            subject = value->data();
                    delete collectableKey;
                    delete value ;

                    errorMsg = "&nbsp;";
                }
                else
                {
                    subject = m_msgSubject ;
                }

                // Display the form for editing the message.
                dynamicHtml +=  "<form action=\"" + redirectUrl + "\">\n " \
                                "<table width=\"85%\" border=\"0\">\n" \
                                "<tr>\n" \
                                  "<td class=\"formtitle\" height=\"30\" width=\"92%\">Contents of <i>" + m_folderName + "</i></td>\n" \
                                  "<td align=\"right\" width=\"8%\">&nbsp;<a class=\"bgdark\" href=\"javascript:void 0\" onclick=\"displayHelpPage('/userui/WebHelp/mediauser.htm#all_folders_managing_messages.htm');\">Help</a></td>\n" \
                                "</tr>\n" \
                                "<tr>\n" \
                                  "<td class=\"errortext_light\" colspan=\"2\">\n" \
                                    "<hr class=\"dms\">\n" \
                                  "</td>\n" \
                                "</tr>\n" \
                                "<tr> \n" \
                                  "<td class=\"errortext_light\" colspan=\"2\"> \n" + errorMsg + "</td>\n" \
                                "</tr>\n" \
                                "<tr> \n" \
                                  "<td colspan=\"2\"> \n" \
                                    "<table border=\"0\" align=\"left\" width=\"100%\"> \n" \
                                      "<tr> \n" +
                                        "<td colspan=\"2\" height=\"57\"> \n" \
                                          "<table class=\"bglist\" cellspacing=\"1\" cellpadding=\"4\" border=\"0\" width=\"100%\">\n" \
                                            "<tr> \n" \
                                              "<th colspan=\"2\">Edit Message</th>\n" \
                                            "</tr> \n" \
                                            "<tr> \n" +
                                                "<td width=\"24%\">From:</td>\n" \
                                                "<td width=\"76%\"> " + from + " </td>\n" \
                                            "</tr>\n" \
                                            "<tr> \n" \
                                                "<td width=\"24%\">Date:</td>\n" \
                                                "<td width=\"76%\"> " + timestamp + " </td>\n" \
                                            "</tr>\n" \
                                            "<tr> \n" +
                                                "<td width=\"24%\">Subject:</td>\n" \
                                                "<td width=\"76%\"> \n" \
                                                "<input type=\"text\" name=\"subject\" size=\"30\" value=\"" + subject + "\">\n" \
                                                "</td>\n" \
                                            "</tr>\n" \
                                          "</table>\n" \
                                        "</td> \n" \
                                      "</tr> \n" \
                                      "<tr> \n" \
                                        "<td colspan=\"2\" align=\"left\"> \n" \
                                            "<table cellspacing=\"3\" cellpadding=\"3\"> \n" \
                                                "<tr> \n" \
                                                    "<td align=\"left\"> \n" \
                                                                        "<input type=\"hidden\" name=\"foldername\" value=\"" + m_folderName + "\">\n" \
                                                        "<input type=\"hidden\" name=\"messageid\" value=\"" + m_messageId + "\">\n" \
                                                        "<input type=\"hidden\" name=\"formsubmitted\" value=\"yes\">\n" \
                                                        "<input type=\"hidden\" name=\"fromweb\" value=\"yes\">\n" \
                                                        "<input type=\"hidden\" name=\"action\" value=\"editmsg\">\n" \
                                                        "<input type=\"hidden\" name=\"nextblockhandle\" value=\"" + m_nextBlockHandle + "\">\n" \
                                                        "<input type=\"submit\" name=\"submit\" value=\"Save\">\n" \
                                                        "</form> \n" \
                                                    "</td> \n" \
                                                    "<td align=\"left\"> \n" \
                                                        "<form method=\"post\" action=\"" + redirectUrl + "\"> \n" \
                                                        "<input type=\"hidden\" name=\"fromweb\" value=\"yes\">\n" \
                                                        "<input type=\"hidden\" name=\"action\" value=\"playmsg\">\n" \
                                                        "<input type=\"hidden\" name=\"from\" value=\"gateway\">\n" \
                                                        "<input type=\"hidden\" name=\"nextblockhandle\" value=\"" + m_nextBlockHandle + "\">\n" \
                                                        "<input type=\"hidden\" name=\"category\" value=\"" + m_folderName + "\">\n" \
                                                        "<input type=\"submit\" name=\"submit\" value=\"Cancel\">\n" \
                                                        "</form> \n" \
                                                    "</td> \n" \
                                                "</tr> \n" \
                                            "</table>\n" \
                                        "</td> \n" \
                                    "</tr> \n" \
                                "</table>\n"
                                "</td>\n" \
                                "</tr> \n" \
                                "</table> \n" ;

            }
        }
        else
        {
                    redirectUrl += VOICEMAIL_NOT_ENABLED_URL;
            dynamicHtml += REDIRECT_SCRIPT_BEGIN + redirectUrl + REDIRECT_SCRIPT_END ;
        }
    }
    else
    {
        dynamicHtml =   HTML_BEGIN \
                        PROTOCOL_MISMATCH ;
    }

    dynamicHtml += HTML_END ;

        if (out)
        {
                out->remove(0);
                out->append(dynamicHtml.data());
        }

        return OS_SUCCESS ;
}
