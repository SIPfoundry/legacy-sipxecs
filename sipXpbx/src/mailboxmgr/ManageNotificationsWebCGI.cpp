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
#include "mailboxmgr/ManageNotificationsWebCGI.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlSortedList.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

ManageNotificationsWebCGI::ManageNotificationsWebCGI(   const UtlString& mailboxIdentity,
                                                        const UtlString& action,
                                                        const UtlString& status,
                                                        const UtlString& contactAddress,
                                                        const UtlString& contactType,
                                                        const UtlString& newContactAddress,
                                                        const UtlString& newContactType,
                                                        const UtlString& sendAttachments) :
    m_mailboxIdentity ( mailboxIdentity),
    m_action( action ),
    m_status( status ),
    m_contactAddress ( contactAddress ),
    m_contactType( contactType ),
    m_newContactAddress ( newContactAddress ),
    m_newContactType( newContactType ),
    m_sendAttachments ( sendAttachments )

{}

ManageNotificationsWebCGI::~ManageNotificationsWebCGI()
{}

OsStatus
ManageNotificationsWebCGI::execute(UtlString* out)
{

        // Validate the mailbox id and extension.
    ValidateMailboxCGIHelper validateMailboxHelper ( m_mailboxIdentity );
    OsStatus result = validateMailboxHelper.execute( out );
    if ( result == OS_SUCCESS )
    {
        // Get the base URL for the Mediaserver
        MailboxManager* pMailboxManager = MailboxManager::getInstance();
        result = pMailboxManager->getMediaserverURLForWeb( m_cgiUrl ) ;
        if( result == OS_SUCCESS )
        {
            // Construct the base URL for the CGI
            m_cgiUrl += CGI_URL ;

            // Get the actual mailbox identity.
            validateMailboxHelper.getMailboxIdentity( m_mailboxIdentity );

            // Call appropriate methods based on the action parameter
            if( m_action == "managenotifications" )
            {
                result = getManageNotificationsUI(out) ;
            }
            else if( m_action == "geteditnotificationui" ||
                     m_action == "getaddnotificationui" )
            {
                result = getAddEditNotificationsUI(out);
            }
            else if(m_action == "addnotification" ||
                    m_action == "editnotification" ||
                    m_action == "deletenotification" )
            {
                result = addEditDeleteNotification( out );
            }
            else
            {
                result = OS_FAILED ;
            }
        }
        else
        {
            UtlString dynamicHtml =  HTML_BEGIN \
                                    PROTOCOL_MISMATCH \
                                    HTML_END ;
            if( out )
            {
                out->remove(0);
                out->append( dynamicHtml.data() );
            }

            result = OS_SUCCESS ;
        }

    }

    return result ;
}


OsStatus
ManageNotificationsWebCGI::getManageNotificationsUI(UtlString* out)
{
    // Construct the status message
    UtlString statusStr = "&nbsp";
    if( m_status == ADD_NOTIFICATION_SUCCESS )
        statusStr = "Email address added successfully" ;
    else if( m_status == EDIT_NOTIFICATION_SUCCESS )
        statusStr = "Email address modified successfully" ;
    else if( m_status == DELETE_NOTIFICATION_SUCCESS )
        statusStr = "Email address deleted successfully" ;
    else if( m_status == DELETE_NOTIFICATION_FAILED )
        statusStr = "Failed to delete the email address. Please try again." ;

    UtlString dynamicHtml =  HTML_BEGIN \
                            WEBPAGE_SNIPPET1 \
                            "Manage Notifications" \
                            WEBPAGE_SNIPPET2 \
                            "notify_by_email_page.htm" \
                            WEBPAGE_SNIPPET3 ;


    MailboxManager* pMailboxManager = MailboxManager::getInstance();

    if( TRUE ) // email notification is always enabled now
    {
        // Get the list of contacts
        UtlBoolean contactsFound = FALSE ;

        UtlHashMap contactsHashDictionary;
        pMailboxManager->getNotificationContactList (
            m_mailboxIdentity,
            &contactsHashDictionary );

        if( contactsHashDictionary.entries() > 0 )
        {
            // Get the sorted contacts list
            // Get the sorted list of contacts.
            UtlSortedList* contactList = (UtlSortedList*) contactsHashDictionary.
                    findValue( new UtlString("sortedcontacts") );

            if (contactList != NULL)
            {
                if( contactList->entries() > 0)
                {
                    contactsFound = TRUE ;

                    // Add code for displaying the status and
                    // the table header for listing the contacts.
                    dynamicHtml +=  statusStr +
                                WEBPAGE_SNIPPET4 +
                                "<tr> \n" \
                                  "<th>Email Address</th>\n" \
                                  "<th width=\"15%\">Edit</th>\n" \
                                  "<th width=\"15%\">Delete</th>\n" \
                                "</tr>\n" ;
                }

                while( contactList->entries() > 0 )
                {

                    UtlString* rwAddress =
                        (UtlString*) contactList->removeAt(0);
                    UtlString address = rwAddress->data();

                    if( contactsHashDictionary.findValue( rwAddress ) != NULL )
                    {
                        UtlHashMap* contactDetails = (UtlHashMap*) contactsHashDictionary.
                                                findValue( rwAddress );

                        // Construct the HTML code
                        UtlString htmlCode ;
                        getNotificationsUIHtmlCode( address, contactDetails, htmlCode ) ;
                        dynamicHtml += htmlCode ;
                    }

                }
                delete contactList ;
            }
        }

        if( !contactsFound )
        {
            statusStr += "<br><br>Notifications can be sent to your email when you receive a new voicemail.<br>Click on the button below to specify your email address";
            dynamicHtml += statusStr + "</td></tr>" ;
        }

        dynamicHtml +=  "</table>\n" \
                        "</td></tr>\n" \
                        "<tr>\n" \
                            "<td colspan=\"2\"> \n" \
                               "<form action=\"" + m_cgiUrl + "\" method=\"post\">\n" \
                               "<input type=\"hidden\" name=\"action\" value=\"getaddnotificationui\">\n" \
                               "<input type=\"hidden\" name=\"fromweb\" value=\"yes\">\n" \
                               "<input type=\"submit\" value=\"Add Notification\">\n" \
                               "</form>\n" \
                            "</td> \n" \
                        "</tr> \n" ;
    }
    else
    {
        dynamicHtml +=  "<br><br>Email notification feature has not been enabled. Please contact your SIPxchange administrator." \
                        "</table>\n" \
                        "</td></tr>\n" ;
    }

    dynamicHtml +=  "</table>\n" \
                    "</td>\n" \
                    "</tr>\n" \
                    "</table>\n" \
                    HTML_END ;

    if( out )
    {
        out->remove(0);
        out->append( dynamicHtml.data() );
    }

    return OS_SUCCESS ;
}

OsStatus
ManageNotificationsWebCGI::getNotificationsUIHtmlCode(  const UtlString& contactAddress,
                                                        UtlHashMap* contactDetailsHashDict,
                                                        UtlString& htmlCode) const
{
    UtlString* rwContactType = (UtlString*) contactDetailsHashDict->
                                findValue( new UtlString("type") );
    UtlString* rwSendAttachments = (UtlString*) contactDetailsHashDict->
                                findValue( new UtlString("attachments") );

    UtlString contactType = rwContactType->data();
    UtlString sendAttachments = rwSendAttachments->data();

    UtlString redirectUrl    = m_cgiUrl + "?fromweb=yes&contact=" + contactAddress + "&type=" + contactType + "&attachments=" + sendAttachments + "&action=" ;
    UtlString editColumn     = "<a href=\"" + redirectUrl + "geteditnotificationui\"><img src=\"/images/editicon.gif\" width=\"12\" height=\"12\" border=\"0\"></a>";
    UtlString deleteColumn   = "<a href=\"" + redirectUrl + "deletenotification\" onClick=\"return confirm('" + DELETE_CONFIRMATION + "');\"><img src=\"/images/del.gif\" width=\"12\" height=\"12\" border=\"0\"></a>";

    htmlCode =  "<tr> \n" \
                          "<td>" + contactAddress + "</td> \n" \
                          "<td align=\"center\">" + editColumn + "</td> \n" \
                          "<td align=\"center\">" + deleteColumn + "</td> \n" \
                        "</tr> \n" ;

    delete rwContactType ;
    delete rwSendAttachments ;

    return OS_SUCCESS ;
}

OsStatus
ManageNotificationsWebCGI::getAddEditNotificationsUI(UtlString* out) const
{
    // Construct the status message text
    UtlString statusStr = "&nbsp";
    if( m_status == ADD_NOTIFICATION_FAILED )
        statusStr = "Failed to add new email address. It is invalid or is already available." ;
    else if( m_status == EDIT_NOTIFICATION_FAILED )
        statusStr = "Failed to edit email address." ;
    else if( m_status == NOTIFICATION_DUPLICATE_FOUND )
        statusStr = "Please enter a unique email address" ;
    else if( m_status == INVALID_NOTIFICATION_ADDRESS )
        statusStr = "Please enter a valid email address" ;


    // Title to be displayed on the table header
    UtlString title ;
    if( m_action == "geteditnotificationui" )
        title = "Edit Email Address <i>" + m_contactAddress + "</i>" ;
    else
        title += "Add Email Address" ;

    // Value to pre-populate the 'Contact' text field
    UtlString contactValue ;
    if( m_newContactAddress.compareTo("-1") != 0 )
        contactValue = m_newContactAddress ;
    else if( m_contactAddress.compareTo("-1") != 0 )
        contactValue = m_contactAddress;
    else
        contactValue = "" ;

    // Populate the 'Enable attachments' checkbox.
    UtlString attachments = "<input type=\"checkbox\" name=\"attachments\" value=\"yes\"" ;
    if( m_sendAttachments.compareTo("yes") == 0 )
            attachments += " CHECKED" ;
    attachments += ">" ;


    // Value of the action parameter to be taken when the form is submitted
    UtlString submitAction ;
    if( m_action == "geteditnotificationui" )
        submitAction = "editnotification" ;
    else
        submitAction = "addnotification" ;

    // Construct the HTML code
    UtlString dynamicHtml =  HTML_BEGIN \
                            "<form action=\"" + m_cgiUrl + "\">\n " +
                            WEBPAGE_SNIPPET1 +
                            "Manage Notifications" +
                            WEBPAGE_SNIPPET2 +
                            "notify_by_email_page.htm" +
                            WEBPAGE_SNIPPET3 +
                            statusStr +
                            WEBPAGE_SNIPPET4 +
                            "<tr> \n" +
                                "<th colspan=\"2\">" + title + "</th>\n" \
                            "</tr>\n" \
                            "<tr> " \
                                "<td width=\"39%\">Email Address</td> \n" \
                                "<td width=\"61%\"> \n" \
                                    "<input type=\"text\" name=\"newcontact\" size=\"40\" value=\"" + contactValue + "\" >\n" \
                                "</td> \n" \
                            "</tr> \n" \
                            "<tr> " \
                                "<td width=\"39%\">Attach voice message</td> \n" \
                                "<td width=\"61%\"> \n" + attachments + "</td> \n" \
                            "</tr> \n" \
                            "</table> \n" \
                            "</td></tr> \n" \
                            "<tr> \n" \
                                "<td colspan=\"2\" align=\"left\"> \n" \
                                    "<table cellspacing=\"3\" cellpadding=\"3\"> \n" \
                                        "<tr> \n" \
                                            "<td align=\"left\"> \n" \
                                                "<input type=\"hidden\" name=\"action\" value=\"" + submitAction + "\"> \n" \
                                                "<input type=\"hidden\" name=\"fromweb\" value=\"yes\"> \n" \
                                                "<input type=\"hidden\" name=\"contact\" value=\"" + m_contactAddress + "\"> \n" \
                                                "<input type=\"submit\" name=\"Submit\" value=\"Save\">" \
                                                "</form> \n" \
                                            "</td> \n" \
                                            "<td align=\"left\"> \n" \
                                                "<form action=\"" + m_cgiUrl + "\" method=\"post\"> \n" \
                                                "<input type=\"submit\" name=\"Submit2\" value=\"Cancel\"> \n" \
                                                "<input type=\"hidden\" name=\"fromweb\" value=\"yes\"> \n" \
                                                "<input type=\"hidden\" name=\"action\" value=\"managenotifications\"> \n" \
                                                "</form> \n" \
                                            "</td> \n" \
                                        "</tr> \n" \
                                    "<table> \n" \
                                "</td> \n" \
                            "</tr> \n" \
                            "</table> \n" \
                            "</td>\n" \
                            "</tr> \n" \
                            "</table> \n"
                            HTML_END;

        // Write out the dynamic VXML script to be processed by OpenVXI
        if (out)
        {
                out->remove(0);
                out->append(dynamicHtml.data());
        }

    return OS_SUCCESS ;
}

OsStatus
ManageNotificationsWebCGI::addEditDeleteNotification(UtlString* out) const
{
    OsStatus result = OS_SUCCESS ;

    // For add and edit, first validate the contact address.
    if( m_action == "addnotification" || m_action == "editnotification" )
        result = validateContactAddress() ;

    if( result == OS_SUCCESS )
    {
        MailboxManager* pMailboxManager = MailboxManager::getInstance();
        result = pMailboxManager->
                            addEditDeleteNotification(   m_mailboxIdentity,
                                                         m_action,
                                                         m_contactAddress,
                                                         m_newContactAddress,
                                                         m_newContactType,
                                                         m_sendAttachments);
    }

    // URL of the result webpage
    UtlString redirectUrl =  m_cgiUrl + "?fromweb=yes&status=";
    if( result == OS_SUCCESS )
    {
        UtlString status ;
        if( m_action == "addnotification" )
            status = ADD_NOTIFICATION_SUCCESS ;
        else if( m_action == "editnotification" )
            status = EDIT_NOTIFICATION_SUCCESS ;
        else if( m_action == "deletenotification" )
            status = DELETE_NOTIFICATION_SUCCESS ;

                redirectUrl +=  status + "&action=managenotifications" ;

    }
    else if( result == OS_NAME_IN_USE )
    {
        redirectUrl +=  UtlString( NOTIFICATION_DUPLICATE_FOUND ) ;
        if( m_action == "addnotification" )
                    redirectUrl +=  "&action=getaddnotificationui" ;
        else if( m_action == "editnotification" )
            redirectUrl +=  "&action=geteditnotificationui" ;
    }
    else if( result == OS_INVALID )
    {
        redirectUrl +=  UtlString( INVALID_NOTIFICATION_ADDRESS ) ;
        if( m_action == "addnotification" )
                    redirectUrl +=  "&action=getaddnotificationui" ;
        else if( m_action == "editnotification" )
            redirectUrl +=  "&action=geteditnotificationui" ;

    }
    else
    {
        if( m_action == "addnotification" )
                    redirectUrl +=  UtlString( ADD_NOTIFICATION_FAILED ) + "&action=getaddnotificationui" ;
        else if( m_action == "editnotification" )
            redirectUrl +=  UtlString( EDIT_NOTIFICATION_FAILED ) + "&action=geteditnotificationui" ;
        else if( m_action == "deletenotification" )
            redirectUrl +=  UtlString( DELETE_NOTIFICATION_FAILED ) + "&action=managenotifications" ;
    }

    redirectUrl +=  "&contact=" + m_contactAddress +
                    "&type=" + m_contactType +
                    "&newcontact=" + m_newContactAddress +
                    "&newtype=" + m_newContactType +
                    "&attachments=" + m_sendAttachments;

    // Script for redirecting to the webpage displaying folder contents
        UtlString dynamicHtml  =        HTML_BEGIN \
                            REDIRECT_SCRIPT_BEGIN + redirectUrl + REDIRECT_SCRIPT_END \
                            HTML_END ;

        if (out)
        {
                out->remove(0);
                out->append(dynamicHtml.data());
        }

    return OS_SUCCESS ;
}

OsStatus
ManageNotificationsWebCGI::validateContactAddress() const
{
    OsStatus result = OS_INVALID ;

    // Find the index of '@' in the contact address
    unsigned int at_index        = m_newContactAddress.index( '@' );
    unsigned int last_at_index   = m_newContactAddress.last( '@' );
    unsigned int dot_index       = m_newContactAddress.last( '.' );
    unsigned int lastIndex       = m_newContactAddress.length() - 1;

    // There should be only one '@' sign.
    if( at_index != UTL_NOT_FOUND && at_index == last_at_index )
    {
        // '@' should not be the first or the last character.
        if( at_index != 0 && at_index != lastIndex )
        {
            // There should be atleast one character between @ and .
            if( (dot_index - at_index) > 1 )
            {
                // '.' should not be the first or last char
                if( dot_index != 0 && dot_index != lastIndex )
                {
                    result = OS_SUCCESS ;
                }
            }
        }
    }

    return result ;
}
