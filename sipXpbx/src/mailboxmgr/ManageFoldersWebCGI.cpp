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
#include "mailboxmgr/ManageFoldersWebCGI.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"
#include "net/HttpMessage.h"
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

ManageFoldersWebCGI::ManageFoldersWebCGI( const UtlString& mailboxIdentity,
                                          const UtlString& action,
                                          const UtlString& status,
                                          const UtlString& strNewFolderName,
                                          const UtlString& strOldFolderName ) :
    m_mailboxIdentity ( mailboxIdentity),
    m_action( action ),
    m_status( status ),
    m_newFolder( strNewFolderName ),
    m_oldFolder( strOldFolderName )
{}

ManageFoldersWebCGI::~ManageFoldersWebCGI()
{}

OsStatus
ManageFoldersWebCGI::execute(UtlString* out)
{

        // Validate the mailbox id and extension.
    ValidateMailboxCGIHelper validateMailboxHelper ( m_mailboxIdentity );
    OsStatus result = validateMailboxHelper.execute( out );
    if ( result == OS_SUCCESS )
    {
        MailboxManager* pMailboxManager = MailboxManager::getInstance();
        result = pMailboxManager->getMediaserverURLForWeb( m_cgiUrl ) ;
        if( result == OS_SUCCESS )
        {
            m_cgiUrl += CGI_URL ;

            // unescape the folder names
            HttpMessage::unescape( m_newFolder );
            HttpMessage::unescape( m_oldFolder );

            validateMailboxHelper.getMailboxIdentity( m_mailboxIdentity );

            if( m_action == "getpersonalfolders" )
            {
                result = getPersonalFolders(out) ;
            }
            else if( m_action == "getfolders" )
            {
                result = getAllFolders( out );
            }
            else if(m_action == "addfolder" ||
                    m_action == "editfolder" ||
                    m_action == "deletefolder" )
            {
                result = addEditDeleteFolder( out );
            }
            else if(m_action == "getaddfolderui" ||
                    m_action == "geteditfolderui" )
            {
                result = getAddEditFolderUI( out );
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
ManageFoldersWebCGI::getPersonalFolders(UtlString* out)
{

    UtlString dynamicHtml( HTML_BEGIN );
        UtlString redirectUrl =  m_cgiUrl ;

    // Get the folder names
    UtlSortedList folderList;
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
        pMailboxManager->getMailboxFolders( m_mailboxIdentity, folderList );

    // Check if mailbox has folders other than the standard 3
    if ( folderList.entries() > 3 )
    {
        if( folderList.entries() == 4 )
        {
            // Just one custom folder is available.
            // Redirect to the CGI for displaying contents of the folder
            while( folderList.entries() > 0 )
            {
                UtlString* rwFolderName =
                    (UtlString*) folderList.removeAt(0);
                UtlString folderName = rwFolderName->data();
                delete rwFolderName ;

                if( (folderName.compareTo("inbox", UtlString::ignoreCase) != 0) &&
                    (folderName.compareTo("saved", UtlString::ignoreCase) != 0) &&
                    (folderName.compareTo("deleted", UtlString::ignoreCase) != 0)
                )
                {
                    // escape the folder name
                    UtlString escapedFolderName = folderName;
                    HttpMessage::escape( escapedFolderName );

                    redirectUrl += "?action=playmsg&fromweb=yes&from=gateway@pingtel.com&nextblockhandle=-1&category=" + escapedFolderName;
                    break;
                }
            }

            dynamicHtml +=  REDIRECT_SCRIPT_BEGIN + redirectUrl + REDIRECT_SCRIPT_END \
                            HTML_END ;
        }
        else
        {
            dynamicHtml += "<form action=\"" + redirectUrl + "\">\n " +
                            WEBPAGE_SNIPPET1 +
                            "Personal Folders" +
                            WEBPAGE_SNIPPET2 +
                            "all_folders_managing_messages.htm" +
                            WEBPAGE_SNIPPET3 +
                            "Select the folder you want to view" +
                            "</td></tr> \n"+
                            "<tr> \n" \
                                "<td colspan=\"2\" class=\"notetext\"> \n" ;

            while( folderList.entries() > 0 )
            {
                // Show the list of folders so that the user can pick the one they want
                UtlString* rwFolderName =
                    (UtlString*) folderList.removeAt(0);
                UtlString folderName = rwFolderName->data();
                delete rwFolderName ;

                if( (folderName.compareTo("inbox", UtlString::ignoreCase) != 0) &&
                    (folderName.compareTo("saved", UtlString::ignoreCase) != 0) &&
                    (folderName.compareTo("deleted", UtlString::ignoreCase) != 0)
                )
                {
                    UtlString escapedFolderName = folderName ;
                    HttpMessage::escape(escapedFolderName);
                    dynamicHtml += "<input type=\"radio\" name=\"category\" value=\"" + escapedFolderName + "\">" + folderName + "<br>\n" ;
                }
            }

            dynamicHtml += "</td></tr>\n" \
                           "</table>\n" \
                           "<input type=\"hidden\" name=\"action\" value=\"playmsg\">\n" \
                           "<input type=\"hidden\" name=\"fromweb\" value=\"yes\">\n" \
                           "<input type=\"hidden\" name=\"from\" value=\"gateway@pingtel.com\">\n" \
                           "<input type=\"hidden\" name=\"nextblockhandle\" value=\"-1\">\n" \
                           "<input type=\"submit\" value=\"View\">\n" \
                           "</form>\n" \
                           HTML_END ;

        }
    }
    else
    {
        // User does not have any personal folder.
        dynamicHtml +=  WEBPAGE_SNIPPET1 \
                        "Personal Folders" \
                        WEBPAGE_SNIPPET2 \
                        "all_folders_managing_messages.htm" \
                        WEBPAGE_SNIPPET3 \
                        "<br>You do not have any personal folders set up.<br><br>To add a personal folder, click Manage Folders." \
                        "</td></tr>\n" \
                        "</table> \n" \
                        HTML_END ;
    }

    if( out )
    {
        out->remove(0);
        out->append( dynamicHtml.data() );
    }

    return OS_SUCCESS ;
}

OsStatus
ManageFoldersWebCGI::getAllFolders(UtlString* out)
{

    UtlString statusStr = "&nbsp";
    if( m_status == ADD_FOLDER_SUCCESS )
        statusStr = "Folder added successfully" ;
    else if( m_status == EDIT_FOLDER_SUCCESS )
        statusStr = "Folder edited successfully" ;
    else if( m_status == DELETE_FOLDER_SUCCESS )
        statusStr = "Folder deleted successfully" ;
    else if( m_status == DELETE_FOLDER_FAILED )
        statusStr = "Failed to delete folder. Please try again." ;

    // Get the folder names
    UtlSortedList folderList;
    MailboxManager* pMailboxManager = MailboxManager::getInstance();
        pMailboxManager->getMailboxFolders( m_mailboxIdentity, folderList );

    // Check if mailbox has folders other than the standard 3
    UtlString dynamicHtml =  HTML_BEGIN \
                            WEBPAGE_SNIPPET1 \
                            "Manage Folders" \
                            WEBPAGE_SNIPPET2 \
                            "manage_folders_page.htm" \
                            WEBPAGE_SNIPPET3 ;


    UtlString personalFolders ;
    bool personalFoldersFound = false ;

    while( folderList.entries() > 0 )
    {
        // Show the list of folders so that the user can pick the one they want
        UtlString* rwFolderName =
            (UtlString*) folderList.removeAt(0);
        UtlString folderName = rwFolderName->data();
        delete rwFolderName ;

        UtlString editStr = "&nbsp;" ;
        UtlString deleteStr = "&nbsp;" ;

        if( (folderName.compareTo("inbox", UtlString::ignoreCase) != 0) &&
            (folderName.compareTo("saved", UtlString::ignoreCase) != 0) &&
            (folderName.compareTo("deleted", UtlString::ignoreCase) != 0)
        )
        {
            personalFoldersFound = true;

            // escape the folder name
            UtlString escapedFolderName = folderName ;
            HttpMessage::escape( escapedFolderName );

            UtlString redirectUrl =
                m_cgiUrl + "?fromweb=yes&oldfoldername=" + escapedFolderName + "&action=" ;

            editStr = "<a href=\"" + redirectUrl + "geteditfolderui\"><img src=\"/images/editicon.gif\" width=\"12\" height=\"12\" border=\"0\"></a>";
            deleteStr = "<a href=\"" + redirectUrl + "deletefolder\" onClick=\"return confirm('" + DELETE_CONFIRMATION + "');\"><img src=\"/images/del.gif\" width=\"12\" height=\"12\" border=\"0\"></a>";

            if( personalFolders.isNull() )
            {
                personalFolders =   statusStr +
                                    WEBPAGE_SNIPPET4 +
                                    "<tr> \n" \
                                      "<th>Personal Folder</th>\n" \
                                      "<th width=\"15%\">Edit</th>\n" \
                                      "<th width=\"15%\">Delete</th>\n" \
                                    "</tr>\n" ;
            }
            personalFolders += "<tr> \n" \
                                  "<td>" + folderName + "</td> \n" \
                                  "<td align=\"center\">" + editStr + "</td> \n" \
                                  "<td align=\"center\">" + deleteStr + "</td> \n" \
                                "</tr> \n" ;

        }
    }

    if( !personalFoldersFound )
    {
        statusStr += "<br><br>There are no personal folders associated with this mailbox.";
        dynamicHtml += statusStr + "</td></tr>" ;
    }
    else
    {
        dynamicHtml +=  personalFolders ;
    }

    dynamicHtml +=  "</table>\n" \
                    "</td></tr>\n" \
                    "<tr>\n" \
                        "<td colspan=\"2\"> \n" \
                           "<form action=\"" + m_cgiUrl + "\" method=\"post\">\n" \
                           "<input type=\"hidden\" name=\"action\" value=\"getaddfolderui\">\n" \
                           "<input type=\"hidden\" name=\"fromweb\" value=\"yes\">\n" \
                           "<input type=\"submit\" value=\"Create Folder\">\n" \
                           "</form>\n" \
                        "</td> \n" \
                    "</tr> \n" \
                    "</table>\n" \
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
ManageFoldersWebCGI::addEditDeleteFolder(UtlString* out)
{
    // URL of the result webpage
    UtlString redirectUrl =  m_cgiUrl + "?fromweb=yes&status=";

    MailboxManager* pMailboxManager = MailboxManager::getInstance();
    OsStatus result = OS_FAILED ;
    if( m_action == "addfolder" )
    {
        result = pMailboxManager->addFolder( m_mailboxIdentity,
                                             m_newFolder );

            if( result == OS_SUCCESS )
                    redirectUrl +=  UtlString( ADD_FOLDER_SUCCESS ) + "&action=getfolders" ;
        else
        {
            UtlString escapedNewFolder = m_newFolder ;
            HttpMessage::escape( escapedNewFolder );
            redirectUrl +=  UtlString( ADD_FOLDER_FAILED ) + "&action=getaddfolderui&newfoldername=" + escapedNewFolder ;
        }

        result = OS_SUCCESS ;
    }
    else if( m_action == "deletefolder" )
    {
        result = pMailboxManager->deleteFolder( m_mailboxIdentity,
                                                m_oldFolder );
            if( result == OS_SUCCESS )
                    redirectUrl +=  UtlString( DELETE_FOLDER_SUCCESS ) + "&action=getfolders" ;
        else
            redirectUrl +=  UtlString( DELETE_FOLDER_FAILED ) + "&action=getfolders";

        result = OS_SUCCESS ;
    }
    else if( m_action == "editfolder" )
    {
        result = pMailboxManager->editFolder(   m_mailboxIdentity,
                                                m_oldFolder,
                                                m_newFolder );

        // Add status to indicate that the move operation failed
            if( result == OS_SUCCESS )
                    redirectUrl +=  UtlString( EDIT_FOLDER_SUCCESS ) + "&action=getfolders";
        else
        {
            UtlString escapedNewFolder = m_newFolder ;
            HttpMessage::escape( escapedNewFolder );
            UtlString escapedOldFolder = m_oldFolder ;
            HttpMessage::escape( escapedOldFolder );

            redirectUrl +=  UtlString( EDIT_FOLDER_FAILED ) + "&action=geteditfolderui&newfoldername=" + escapedNewFolder + "&oldfoldername=" + escapedOldFolder;
        }

        result = OS_SUCCESS ;
    }

    if( result == OS_SUCCESS )
    {
        // Script for redirecting to the webpage displaying folder contents
            UtlString dynamicHtml  =    HTML_BEGIN \
                                REDIRECT_SCRIPT_BEGIN + redirectUrl + REDIRECT_SCRIPT_END \
                                HTML_END ;

            if (out)
            {
                    out->remove(0);
                    out->append(dynamicHtml.data());
            }
    }

    return result ;
}


OsStatus
ManageFoldersWebCGI::getAddEditFolderUI(UtlString* out)
{
    UtlString dynamicHtml( HTML_BEGIN_WITH_ONLOAD );

    UtlString statusStr = "&nbsp";
    if( m_status == ADD_FOLDER_FAILED )
        statusStr = "Failed to add new folder. Folder name is invalid or another folder exists by that name." ;
    else if( m_status == EDIT_FOLDER_FAILED )
        statusStr = "Failed to edit folder. Folder name is invalid or another folder exists by that name." ;

    dynamicHtml +=  " ONLOAD=\"document.forms[0].newfoldername.focus();\"> \n" \
                    "<form action=\"" + m_cgiUrl + "\" onsubmit=\"return validateFoldername(this);\">\n " +
                    WEBPAGE_SNIPPET1 +
                    "Manage Folders" +
                    WEBPAGE_SNIPPET2 +
                    "manage_folders_page.htm" +
                    WEBPAGE_SNIPPET3 +
                    statusStr +
                    WEBPAGE_SNIPPET4 +
                    "<tr> \n" ;

    if( m_action == "geteditfolderui" )
        dynamicHtml += "<th colspan=\"2\">Edit Folder <i>" + m_oldFolder + "</i></th>\n</tr>\n" ;
    else
        dynamicHtml += "<th colspan=\"2\">Add Folder</th>\n</tr> \n" ;


    dynamicHtml += "<tr> " \
                    "<td width=\"39%\">Folder Name</td> \n" \
                    "<td width=\"61%\"> \n" \
                        "<input type=\"text\" name=\"newfoldername\" value=\"" ;

    if( m_newFolder.compareTo("-1") != 0 )
        dynamicHtml += m_newFolder ;
    else if( m_oldFolder.compareTo("-1") != 0 )
        dynamicHtml += m_oldFolder;

    dynamicHtml +=      "\" >\n" \
                    "</td> \n" \
                    "</tr> \n" \
                    "</table> \n" \
                    "</td></tr> \n" \
                    "<tr> \n" \
                        "<td colspan=\"2\" align=\"left\"> \n" \
                            "<table cellspacing=\"3\" cellpadding=\"3\"> \n" \
                                "<tr> \n" \
                                    "<td align=\"left\"> \n" \
                                        "<input type=\"hidden\" name=\"action\" value=\"" ;

    if( m_action == "geteditfolderui" )
        dynamicHtml += "editfolder" ;
    else
        dynamicHtml += "addfolder" ;

    dynamicHtml +=                      "\"> \n" \
                                        "<input type=\"hidden\" name=\"fromweb\" value=\"yes\"> \n" \
                                        "<input type=\"hidden\" name=\"oldfoldername\" value=\"" + m_oldFolder + "\"> \n" \
                                        "<input type=\"submit\" name=\"Submit\" value=\"Save\">" \
                                        "</form> \n" \
                                    "</td> \n" \
                                    "<td align=\"left\"> \n" \
                                        "<form action=\"" + m_cgiUrl + "\" method=\"post\"> \n" \
                                        "<input type=\"submit\" name=\"Submit2\" value=\"Cancel\"> \n" \
                                        "<input type=\"hidden\" name=\"fromweb\" value=\"yes\"> \n" \
                                        "<input type=\"hidden\" name=\"action\" value=\"getfolders\"> \n" \
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
