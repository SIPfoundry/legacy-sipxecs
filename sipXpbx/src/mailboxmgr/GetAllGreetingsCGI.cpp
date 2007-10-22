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
#include "mailboxmgr/GetAllGreetingsCGI.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"
#include "mailboxmgr/ActiveGreetingHelper.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

GetAllGreetingsCGI::GetAllGreetingsCGI( const UtlBoolean& requestIsFromWebUI,
                                        const UtlString& mailbox,
                                        const UtlString& status ) :
    m_mailboxIdentity ( mailbox ),
    m_fromWeb ( requestIsFromWebUI ),
    m_status ( status )
{}

GetAllGreetingsCGI::~GetAllGreetingsCGI()
{}

OsStatus
GetAllGreetingsCGI::execute(UtlString* out)
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
GetAllGreetingsCGI::handleOpenVXIRequest(UtlString* out)
{
        bool greetingsAvailable = false;
        UtlString greetingUrl;
        UtlString dynamicVxml = getVXMLHeader() + "<form>\n" + "<block>\n" ;

        MailboxManager* pMailboxManager = MailboxManager::getInstance();

        // Get the standard greeting.
        if( pMailboxManager->getGreetingUrl(m_mailboxIdentity, STANDARD_GREETING, greetingUrl, FALSE ) == OS_SUCCESS )
        {
                dynamicVxml += "<var name=\"standard\" expr=\"'" + greetingUrl + "'\" />\n" ;
                greetingsAvailable = true;
        }
        else
        {
                dynamicVxml += "<var name=\"standard\" expr=\"'-1'\" />\n" ;
        }

        // Get the out of office greeting.
        if( pMailboxManager->getGreetingUrl(m_mailboxIdentity, OUTOFOFFICE_GREETING, greetingUrl, FALSE ) == OS_SUCCESS )
        {
                dynamicVxml += "<var name=\"outofoffice\" expr=\"'" + greetingUrl + "'\" />\n" ;
                greetingsAvailable = true;
        }
        else
        {
                dynamicVxml += "<var name=\"outofoffice\" expr=\"'-1'\" />\n" ;
        }

    // Get the URL of the extended absence greeting
        if( pMailboxManager->getGreetingUrl(m_mailboxIdentity, EXTENDED_ABSENCE_GREETING, greetingUrl, FALSE ) == OS_SUCCESS )
        {
                dynamicVxml += "<var name=\"extendedabs\" expr=\"'" + greetingUrl + "'\" />\n" ;
                greetingsAvailable = true;
        }
        else
        {
                dynamicVxml += "<var name=\"extendedabs\" expr=\"'-1'\" />\n" ;
        }

    // Get the default system greeting.
        if( pMailboxManager->getGreetingUrl(m_mailboxIdentity, DEFAULT_STANDARD_GREETING, greetingUrl, FALSE ) == OS_SUCCESS )
        {
                dynamicVxml += "<var name=\"defaultsystem\" expr=\"'" + greetingUrl + "'\" />\n" ;
                greetingsAvailable = true;
        }
        else
        {
                dynamicVxml += "<var name=\"defaultsystem\" expr=\"'-1'\" />\n" ;
        }

        if( greetingsAvailable )
        {
                dynamicVxml +=  "<var name=\"msgurl\" expr=\"'-1'\" />\n" \
                                                "<var name=\"msgurltype\" expr=\"'-1'\" />\n" \
                                                "<var name=\"result\" expr=\"'success'\" />\n" ;
        } else
        {
                // Get the recorded name.
                if( pMailboxManager->getRecordedName( m_mailboxIdentity, greetingUrl, FALSE ) == OS_SUCCESS )
                {
                        // Play <fred> is not available
                        dynamicVxml +=  "<var name=\"msgurl\" expr=\"'" + greetingUrl + "'\" />\n" \
                                                        "<var name=\"msgurltype\" expr=\"'name'\"/>\n" \
                                                        "<var name=\"result\" expr=\"'success'\" />\n" ;
                }
                else
                {
                        // Neither greeting nor recorded name was found.
                        // Hence play "Extension 123 is not available"
            ValidateMailboxCGIHelper validateMailboxHelper ( m_mailboxIdentity );
            validateMailboxHelper.getUserId ( m_mailboxIdentity, greetingUrl );
            greetingUrl.toLower();

                        dynamicVxml +=  "<var name=\"msgurl\" expr=\"'" + greetingUrl + "'\" />\n" \
                                                        "<var name=\"msgurltype\" expr=\"'extension'\"/>\n" \
                                                        "<var name=\"result\" expr=\"'success'\" />\n" ;
                }
        }

        dynamicVxml +=  "<return namelist=\"standard outofoffice extendedabs defaultsystem msgurl msgurltype\" />\n" \
                                        "</block>\n</form>\n";

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
GetAllGreetingsCGI::handleWebRequest(UtlString* out)
{
    UtlString redirectUrl, dynamicHtml ;

    // Retrieve the mediaserver base URL - https://localhost:8091
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;

    if( result == OS_SUCCESS )
    {
        redirectUrl += CGI_URL ;

            UtlString greetingUrl;

        // Construct the appropriate error message for the status sent
        UtlString errorStr = "&nbsp;" ;
        if( m_status == SET_ACTIVE_GREETING_SUCCESS )
            errorStr = "Active greeting was set successfully" ;
        else if( m_status == SET_ACTIVE_GREETING_FAILED )
            errorStr = "Failed to set the active greeting. Please try again." ;
        else if( m_status == DELETE_GREETING_FAILED )
            errorStr = "Failed to delete the greeting. Please try again." ;

        // Construct the HTML for displaying a list of the greetings.
        dynamicHtml =  HTML_BEGIN \
                        EMBED_MEDIAPLAYER_PLUGIN \
                        "<table width=\"85%\" border=\"0\">\n" \
                        "<tr>\n" \
                          "<td class=\"formtitle\" height=\"30\" width=\"92%\">Manage Greetings</td>\n" \
                          "<td align=\"right\" width=\"8%\">&nbsp;<a class=\"bgdark\" href=\"javascript:void 0\" onclick=\"displayHelpPage('/userui/WebHelp/mediauser.htm#manage_greetings_page.htm');\">Help</a></td>\n" \
                        "</tr>\n" \
                        "<tr>\n" \
                          "<td class=\"errortext_light\" colspan=\"2\">\n" \
                            "<hr class=\"dms\">\n" \
                          "</td>\n" \
                        "</tr> \n" \
                        "<tr> \n" \
                          "<td class=\"errortext_light\" colspan=\"2\">" + errorStr + "</td>\n" \
                        "</tr> \n" \
                        "<tr> \n" \
                          "<td colspan=\"2\"> \n" \
                            "<table border=\"0\" align=\"left\" width=\"100%\">\n" \
                              "<tr>\n" \
                                "<td colspan=\"2\" height=\"57\"> \n" \
                                  "<table class=\"bglist\" cellspacing=\"1\" cellpadding=\"4\" border=\"0\" width=\"100%\">\n" \
                                    "<tr> \n" \
                                      "<th width=\"15%\">Active</th>\n" \
                                      "<th>Greeting</th>\n" \
                                      "<th width=\"15%\">Play</th>\n" \
                                    "</tr>\n" ;


        ActiveGreetingHelper greetingHelper;

        // Active greeting type
        UtlString activeGreetingType ;
        if ( greetingHelper.getActiveGreetingType( m_mailboxIdentity, activeGreetingType ) != OS_SUCCESS )
            activeGreetingType = ACTIVE_GREETING_NOT_SET ;

        UtlString rHtmlContent , rSetActiveGreetingContent;

        // HTML code for displaying
        // "Select active greeting : <drop down list of all greetings with active greeting selected> "
        UtlString setActiveGreetingSnippet = "<tr><td class=\"notetext\"> \n" \
                                            "<form action=\"" + redirectUrl + "\" method=\"post\"> \n" \
                                            "Change Active Greeting: " \
                                            "<img src=\"/images/spacer.gif\" width=\"10\"> \n" \
                                            "<select name=\"greetingtype\">\n" ;

        // Generate the html code for Default greeting
        rHtmlContent = "" ;
        rSetActiveGreetingContent = "" ;
        generateManageGreetingsHtml( ACTIVE_GREETING_NOT_SET,
                                     DEFAULT_STANDARD_GREETING_FILE,
                                     activeGreetingType ,
                                     "Default System Greeting",
                                     redirectUrl,
                                     rHtmlContent,
                                     rSetActiveGreetingContent) ;

        if( !rHtmlContent.isNull() )
            dynamicHtml += rHtmlContent ;

        if( !rSetActiveGreetingContent.isNull() )
            setActiveGreetingSnippet += rSetActiveGreetingContent ;

        // Generate the html code for Standard greeting
        generateManageGreetingsHtml( STANDARD_GREETING,
                                     DEFAULT_STANDARD_GREETING_FILE,
                                     activeGreetingType ,
                                     "Standard",
                                     redirectUrl,
                                     rHtmlContent,
                                     rSetActiveGreetingContent) ;

        if( !rHtmlContent.isNull() )
            dynamicHtml += rHtmlContent ;

        if( !rSetActiveGreetingContent.isNull() )
            setActiveGreetingSnippet += rSetActiveGreetingContent ;


        // Generate the html code for Out of Office greeting
        rHtmlContent = "" ;
        rSetActiveGreetingContent = "" ;
        generateManageGreetingsHtml( OUTOFOFFICE_GREETING,
                                     DEFAULT_OUTOFOFFICE_GREETING_FILE,
                                     activeGreetingType ,
                                     "Out of Office",
                                     redirectUrl,
                                     rHtmlContent,
                                     rSetActiveGreetingContent) ;

        if( !rHtmlContent.isNull() )
            dynamicHtml += rHtmlContent ;

        if( !rSetActiveGreetingContent.isNull() )
            setActiveGreetingSnippet += rSetActiveGreetingContent ;

        // Generate the html code for Extended absence greeting
        rHtmlContent = "" ;
        rSetActiveGreetingContent = "" ;
        generateManageGreetingsHtml( EXTENDED_ABSENCE_GREETING,
                                     DEFAULT_EXTENDED_ABS_GREETING_FILE,
                                     activeGreetingType ,
                                     "Extended Absence",
                                     redirectUrl,
                                     rHtmlContent,
                                     rSetActiveGreetingContent) ;

        if( !rHtmlContent.isNull() )
            dynamicHtml += rHtmlContent ;

        if( !rSetActiveGreetingContent.isNull() )
            setActiveGreetingSnippet += rSetActiveGreetingContent ;

        // Reset
        rHtmlContent = "" ;
        rSetActiveGreetingContent = "" ;


        // Get Recorded Name
        UtlString recordedNameUrl ;
        UtlString playColumnContent = "&nbsp;" ;

        // Hari 12/19/2002 Commented out the code for deleting greetings.
        // This feature will be added when we add the functionality for
        // recording greetings using the UI.
        // For now, there is no way of deleting greetings. They can only be replaced

        //UtlString deleteColumnContent = "&nbsp;" ;

        if( greetingHelper.getRecordedName(m_mailboxIdentity, greetingUrl, TRUE ) == OS_SUCCESS )
            {
            UtlString deleteGreetingUrl = redirectUrl + "?action=deletegreeting&fromweb=yes&greetingtype=" + UtlString( RECORDED_NAME ) ;
            playColumnContent = "<a href=\"javascript:void 0\" onclick=\"playMsgJs('" +greetingUrl + "');\" target=\"hiddenFrame\"><img src=\"/images/spkr.gif\" width=\"20\" height=\"18\" border=\"0\"></a>" ;
            //deleteColumnContent = "<a href=\"" + deleteGreetingUrl + "\" onClick=\"return confirm('" + DELETE_CONFIRMATION + "');\"><img src=\"/images/del.gif\" width=\"12\" height=\"12\" border=\"0\"></a>" ;
            }

        dynamicHtml +=  "<tr> \n" \
                            "<td colspan=\"3\">&nbsp;</td> \n" \
                        "</tr> \n" \
                        "<tr> \n" \
                            "<td align=\"center\">&nbsp;</td> \n" \
                            "<td align=\"left\"><img src=\"/images/spacer.gif\" width=\"10\">Recorded Name</td> \n" \
                            "<td align=\"center\">" + playColumnContent + "</td> \n" \
                        "</tr> \n " ;



        setActiveGreetingSnippet += "</select> \n" \
                                    "<img src=\"/images/spacer.gif\" width=\"10\"> \n" \
                                    "<input type=\"hidden\" name=\"action\" value=\"setactivegreeting\"> \n" \
                                    "<input type=\"hidden\" name=\"fromweb\" value=\"yes\"> \n" \
                                    "<input type=\"submit\" name=\"submit\" value=\"Save\">\n " \
                                    "</form>\n" \
                                    "</td></tr>\n" \
                                    RECORD_GREETINGS_NOTE ;


        dynamicHtml +=  "</table> \n" \
                        "</td> \n" \
                        "</tr> \n" +
                        setActiveGreetingSnippet +
                        "</table>\n" \
                        "</td>\n" \
                        "</tr>\n" \
                        "</table>\n" ;
    }
    else
    {
        dynamicHtml =   HTML_BEGIN \
                        PROTOCOL_MISMATCH ;
    }

    dynamicHtml += HTML_END;

        // Write out the dynamic VXML script to be processed by OpenVXI
    if( out )
    {
        out->remove(0) ;
        out->append( dynamicHtml.data() );
    }
    return OS_SUCCESS ;
}


OsStatus
GetAllGreetingsCGI::generateManageGreetingsHtml( const UtlString& greetingType,
                                                 const UtlString& defaultGreetingFilename,
                                                 const UtlString& activeGreetingType ,
                                                 const UtlString& displayName,
                                                 const UtlString& baseUrl,
                                                 UtlString& rHtmlContent,
                                                 UtlString& rSetActiveGreetingContent) const
{
    ActiveGreetingHelper greetingHelper;
    UtlString greetingUrl ;
    UtlString deleteGreetingUrl = baseUrl + "?action=deletegreeting&fromweb=yes&greetingtype=" + greetingType ;
    UtlString activeColumnContent = "&nbsp;" ;
    UtlString playColumnContent = "&nbsp;" ;

    // Hari 12/19/2002 Commented out the code for deleting greetings.
    // This feature will be added when we add the functionality for
    // recording greetings using the UI.
    // For now, there is no way of deleting greetings. They can only be replaced

    // UtlString deleteColumnContent = "&nbsp;";


    if( greetingHelper.getGreetingUrl(m_mailboxIdentity, greetingType, greetingUrl, TRUE, TRUE ) == OS_SUCCESS )
        {

        playColumnContent = "<a href=\"javascript:void 0\" onclick=\"playMsgJs('" + greetingUrl + "');\" target=\"hiddenFrame\"><img src=\"/images/spkr.gif\" width=\"20\" height=\"18\" border=\"0\"></a>" ;

        // A tick mark is placed next to the active greeting.
        // 'checkActive' is used to save the code for displaying tick mark.

        if( activeGreetingType == greetingType )
            deleteGreetingUrl += "&active=yes" ;

        // check if the returned url is for the default greeting.
        //if( greetingUrl.index(defaultGreetingFilename, 0, UtlString::ignoreCase) == UTL_NOT_FOUND )
        //    deleteColumnContent = "<a href=\"" + deleteGreetingUrl + "\" onClick=\"return confirm('" + DELETE_CONFIRMATION + "');\"><img src=\"/images/del.gif\" width=\"12\" height=\"12\" border=\"0\"></a>" ;

        }

    if( activeGreetingType == greetingType )
    {
        activeColumnContent = "<img src=\"/images/tick.gif\">" ;
        rSetActiveGreetingContent = "<option value=\"" + UtlString( greetingType ) + "\" selected>" + displayName + "</option>\n" ;
    }
    else
    {
        rSetActiveGreetingContent = "<option value=\"" + UtlString( greetingType ) + "\">" + displayName + "</option>\n" ;
    }

    rHtmlContent =  "<tr> \n" \
                        "<td align=\"center\">" + activeColumnContent + "</td> \n" \
                        "<td align=\"left\"><img src=\"/images/spacer.gif\" width=\"10\">" + displayName + "</td> \n" \
                        "<td align=\"center\">" + playColumnContent + "</td> \n" \
                    "</tr> \n " ;

    return OS_SUCCESS ;

}
