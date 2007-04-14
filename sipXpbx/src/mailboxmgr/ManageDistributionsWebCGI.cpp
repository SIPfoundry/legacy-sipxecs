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
#include "os/OsFS.h"
#include "mailboxmgr/ManageDistributionsWebCGI.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"
#include "utl/UtlSortedList.h"


// DEFINES
#define NUM_DIST_LISTS 9

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

ManageDistributionsWebCGI::ManageDistributionsWebCGI(   const UtlString& mailboxIdentity,
                                                        const UtlString& action,
                                                        const UtlString& status,
                                                        const UtlString& index,
                                                        const UtlString& distribution ) :
    m_mailboxIdentity ( mailboxIdentity),
    m_action( action ),
    m_status( status ),
    m_index ( index ),
    m_distribution ( distribution )

{}

ManageDistributionsWebCGI::~ManageDistributionsWebCGI()
{}

OsStatus
ManageDistributionsWebCGI::execute(UtlString* out)
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

         // Get the distribution list file
         UtlString mailboxPath;
         result = pMailboxManager->getMailboxPath(m_mailboxIdentity, mailboxPath);

         m_filename = mailboxPath + OsPathBase::separator + DIST_LIST_NAME;

         // Call appropriate methods based on the action parameter
         if( m_action == "managedistributions" )
         {
            result = getManageDistributionsUI(out) ;
         }
         else if( m_action == "geteditdistributionui" ||
                  m_action == "getadddistributionui" )
         {
            result = getAddEditDistributionsUI(out);
         }
         else if(m_action == "adddistribution" ||
                 m_action == "editdistribution" ||
                 m_action == "deletedistribution" )
         {
            result = addEditDeleteDistribution( out );
         }
         else
         {
            result = OS_FAILED ;
         }
      }
      else
      {
         UtlString dynamicHtml = HTML_BEGIN \
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
ManageDistributionsWebCGI::getManageDistributionsUI(UtlString* out)
{
   OsStatus result = OS_FAILED;

   // Construct the status message
   UtlString statusStr = "&nbsp";
   if( m_status == ADD_DISTRIBUTION_SUCCESS )
      statusStr = "Distribution list added successfully" ;
   else if( m_status == EDIT_DISTRIBUTION_SUCCESS )
      statusStr = "Distribution list modified successfully" ;
   else if( m_status == DELETE_DISTRIBUTION_SUCCESS )
      statusStr = "Distribution list deleted successfully" ;
   else if( m_status == DELETE_DISTRIBUTION_FAILED )
      statusStr = "Failed to delete the distribution list. Please try again." ;

   UtlString dynamicHtml =  HTML_BEGIN \
                            WEBPAGE_SNIPPET1 \
                            "Manage Distributions" \
                            WEBPAGE_SNIPPET2 \
                            "manage_distribution_lists_page.htm" \
                            WEBPAGE_SNIPPET3 ;


   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   // Check if the distribution list file exists or not 
   if( OsFileSystem::exists(m_filename) )
   {

      // Display the table header for listing the contents
      dynamicHtml +=  statusStr +
                      WEBPAGE_SNIPPET4 +
                      "<tr> \n" \
                      "<th width=\"5%\">Index</th>\n" \
                      "<th>Distribution Lists</th>\n" \
                      "<th width=\"10%\">Edit</th>\n" \
                      "<th width=\"10%\">Delete</th>\n" \
                      "</tr>\n" ;

      // Parse the distribution list file
      char distIndex[3];
      for ( int i = 1; i <= NUM_DIST_LISTS; i++ )
      {
         sprintf(distIndex, "%d", i);
         UtlSortedList destinations;
         result = pMailboxManager->parseDistributionFile (m_filename, distIndex, destinations);

         if ( result == OS_SUCCESS )
         {
            // Construct the HTML code
            UtlString htmlCode ;
            getDistributionUIHtmlCode( distIndex, destinations, htmlCode ) ;
            dynamicHtml += htmlCode ;
         }
      }
   }
   else
   {
      statusStr += "<br><br>Distribution lists are used for delivering the messages to multiple parties.<br>Click on the button below to create your distribution lists.";
      dynamicHtml += statusStr + "</td></tr>" ;
   }

   dynamicHtml +=  "</table>\n" \
                   "</td></tr>\n" \
                   "<tr>\n" \
                       "<td colspan=\"2\"> \n" \
                           "<form action=\"" + m_cgiUrl + "\" method=\"post\">\n" \
                           "<input type=\"hidden\" name=\"action\" value=\"getadddistributionui\">\n" \
                           "<input type=\"hidden\" name=\"fromweb\" value=\"yes\">\n" \
                           "<input type=\"submit\" value=\"Add Distribution\">\n" \
                           "</form>\n" \
                       "</td> \n" \
                   "</tr> \n" ;

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
ManageDistributionsWebCGI::getDistributionUIHtmlCode(  const UtlString& indexName,
                                                       UtlSortedList& distList,
                                                       UtlString& htmlCode) const
{
   UtlString listValue;

   // Loop through the distribution list
   while (distList.entries() > 0 )
   {
      UtlString* pMailbox = (UtlString *) distList.removeAt(0);
      UtlString mailbox = pMailbox->data();
      delete pMailbox;

      listValue += mailbox + ", ";
   }

   UtlString redirectUrl    = m_cgiUrl + "?fromweb=yes&index=" + indexName + "&distribution=" + listValue + "&action=" ;
   UtlString editColumn     = "<a href=\"" + redirectUrl + "geteditdistributionui\"><img src=\"/images/editicon.gif\" width=\"12\" height=\"12\" border=\"0\"></a>";
   UtlString deleteColumn   = "<a href=\"" + redirectUrl + "deletedistribution\" onClick=\"return confirm('" + DELETE_CONFIRMATION + "');\"><img src=\"/images/del.gif\" width=\"12\" height=\"12\" border=\"0\"></a>";

   htmlCode =  "<tr> \n" \
               "<td align=\"center\">" + indexName + "</td> \n" \
               "<td>" + listValue + "</td> \n" \
               "<td align=\"center\">" + editColumn + "</td> \n" \
               "<td align=\"center\">" + deleteColumn + "</td> \n" \
               "</tr> \n" ;

    return OS_SUCCESS ;
}

OsStatus
ManageDistributionsWebCGI::getAddEditDistributionsUI(UtlString* out) const
{
   // Construct the status message text
   UtlString statusStr = "&nbsp";
   if( m_status == ADD_DISTRIBUTION_FAILED )
      statusStr = "Failed to create new distribution list. Please check your list and try it again." ;
   else if( m_status == EDIT_DISTRIBUTION_FAILED )
      statusStr = "Failed to modify the distribution list. Please check your list and try it again." ;
   else if( m_status == DISTRIBUTION_DUPLICATE_FOUND )
      statusStr = "The number for the distribution list is already being used. Please select another number." ;
   else if( m_status == INVALID_DISTRIBUTION_ADDRESS )
      statusStr = "Please only use a single digit [1 - 9] for the distribution list." ;
   else
      statusStr = "Please use comma to separate each mailbox address in the distribution list." ;


   // Value to pre-populate the 'Index' text field
   UtlString indexValue ;
   if( m_index.compareTo("-1") != 0 )
      indexValue = m_index ;
   else
      indexValue = "" ;

   // Value to pre-populate the 'Distribution' text field
   UtlString inputValue ;
   if( m_distribution.compareTo("-1") != 0 )
      inputValue = m_distribution ;
   else
      inputValue = "" ;

   // Title to be displayed on the table header
   UtlString title ;
   if( m_action == "geteditdistributionui" )
      title = "Edit Distribution List <i>" + m_index + "</i>" ;
   else
      title += "Create New Distribution List <input type=\"text\" name=\"index\" size=\"2\" value=\"" + indexValue +"\" >\n" ;

   // Value of the action parameter to be taken when the form is submitted
   UtlString submitAction ;
   if( m_action == "geteditdistributionui" )
      submitAction = "editdistribution" ;
   else
      submitAction = "adddistribution" ;

   // Construct the HTML code
   UtlString dynamicHtml =  HTML_BEGIN \
                            "<form action=\"" + m_cgiUrl + "\">\n " +
                            WEBPAGE_SNIPPET1 +
                            "Manage Distributions" +
                            WEBPAGE_SNIPPET2 +
                            "manage_distribution_lists_page.htm" +
                            WEBPAGE_SNIPPET3 +
                            statusStr +
                            WEBPAGE_SNIPPET4 +
                            "<tr> \n" +
                                "<th colspan=\"2\">" + title + "</th>\n" \
                            "</tr>\n" \
                            "<tr> " \
                                "<td width=\"5%\">Destinations</td> \n" \
                                "<td width=\"95%\"> \n" \
                                    "<input type=\"text\" name=\"distribution\" size=\"100\" value=\"" + inputValue + "\" >\n" \
                                "</td> \n" \
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
                                                "<input type=\"hidden\" name=\"index\" value=\"" + indexValue + "\"> \n" \
                                                "<input type=\"submit\" name=\"Submit\" value=\"Save\">" \
                                                "</form> \n" \
                                            "</td> \n" \
                                            "<td align=\"left\"> \n" \
                                                "<form action=\"" + m_cgiUrl + "\" method=\"post\"> \n" \
                                                "<input type=\"submit\" name=\"Submit2\" value=\"Cancel\"> \n" \
                                                "<input type=\"hidden\" name=\"fromweb\" value=\"yes\"> \n" \
                                                "<input type=\"hidden\" name=\"action\" value=\"managedistributions\"> \n" \
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
ManageDistributionsWebCGI::addEditDeleteDistribution(UtlString* out) const
{
   OsStatus result = OS_SUCCESS ;

   result = saveDistribution() ;

   // URL of the result webpage
   UtlString redirectUrl =  m_cgiUrl + "?fromweb=yes&status=";
   if( result == OS_SUCCESS )
   {
      UtlString status ;
      if( m_action == "adddistribution" )
         status = ADD_DISTRIBUTION_SUCCESS ;
      else if( m_action == "editdistribution" )
         status = EDIT_DISTRIBUTION_SUCCESS ;
      else if( m_action == "deletedistribution" )
         status = DELETE_DISTRIBUTION_SUCCESS ;

      redirectUrl +=  status + "&action=managedistributions" ;

   }
   else if( result == OS_NAME_IN_USE )
   {
      redirectUrl +=  UtlString( DISTRIBUTION_DUPLICATE_FOUND ) ;
      if( m_action == "adddistribution" )
         redirectUrl +=  "&action=getadddistributionui" ;
      else if( m_action == "editdistribution" )
         redirectUrl +=  "&action=geteditdistributionui" ;
   }
   else if( result == OS_INVALID )
   {
      redirectUrl +=  UtlString( INVALID_DISTRIBUTION_ADDRESS ) ;
      if( m_action == "adddistribution" )
         redirectUrl +=  "&action=getadddistributionui" ;
   }
   else
   {
      if( m_action == "adddistribution" )
         redirectUrl +=  UtlString( ADD_DISTRIBUTION_FAILED ) + "&action=getadddistributionui" ;
      else if( m_action == "editdistribution" )
         redirectUrl +=  UtlString( EDIT_DISTRIBUTION_FAILED ) + "&action=geteditdistributionui" ;
      else if( m_action == "deletedistribution" )
         redirectUrl +=  UtlString( DELETE_DISTRIBUTION_FAILED ) + "&action=managedistributions" ;
   }

   redirectUrl +=  "&index=" + m_index +
                   "&distribution=" + m_distribution;

   // Script for redirecting to the webpage displaying folder contents
   UtlString dynamicHtml =  HTML_BEGIN \
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
ManageDistributionsWebCGI::saveDistribution() const
{
   OsStatus result = OS_FAILED ;

   // Check whether the index exceeds the maximum allowed
   if ( m_index.isNull() || atoi(m_index) > NUM_DIST_LISTS || atoi(m_index) <= 0 )
      return OS_INVALID;

   MailboxManager* pMailboxManager = MailboxManager::getInstance();
   UtlSortedList destinations;
   UtlSortedList oldDestinations;
   UtlString mailbox;
   UtlString indexName;

   // Check whether the index already exists
   if( m_action == "adddistribution" )
   {
      result = pMailboxManager->parseDistributionFile (m_filename, m_index, destinations);
      
      if ( result == OS_SUCCESS )
         return OS_NAME_IN_USE;
   }

   if( m_action == "adddistribution" || m_action == "editdistribution" )
   {
      // Put the distribution list into the sorted list
      int strLength = m_distribution.length();
      char *pBuffer = new char[strLength];
      if (pBuffer)
      {
         memcpy(pBuffer, m_distribution.data(), strLength);
      }

      int i, j;
      char address[10];
      j = 0;
      address[0] = '\0';
      for ( i = 0; i < strLength; i++ )
      {
         if (pBuffer[i] != ' ')
         {
            if (pBuffer[i] != ',')
            {
               address[j] = pBuffer[i];
               j++;
            }
            else
            {
               address[j] = '\0';
               if (validateAddress( address ) == OS_SUCCESS)
               {
                  destinations.insert( new UtlString (address) );
               }
               else
               {
                  OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                                "ManageDistributionsWebCGI - saveDistribution:: invalide address = %s\n",
                                address);

                  return OS_FAILED;
               }

               address[0] = '\0';
               j = 0;
            }
         }
      }

      if (j != 0)
      {
         address[j] = '\0';
         if (validateAddress( address ) == OS_SUCCESS)
         {
            destinations.insert( new UtlString (address) );
         }
         else
         {
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                          "ManageDistributionsWebCGI - saveDistribution:: invalide address = %s\n",
                          address);
            return OS_FAILED;
         }
      }

      delete [] pBuffer;
   }
      
   // Add the distribution list to the file
   UtlString distributionData =
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n" \
      "<distributions>\n";

   char distIndex[3];
   for ( int i = 1; i <= NUM_DIST_LISTS; i++ )
   {
      sprintf(distIndex, "%d", i);
      indexName = distIndex;

      // Check whether the index is the one to be replaced
      if (m_index.compareTo(indexName) != 0 )
      {
         result = pMailboxManager->parseDistributionFile (m_filename, indexName, oldDestinations);

         if ( result == OS_SUCCESS )
         {
            distributionData += "  <list>\n";
            distributionData += "    <index>" + indexName + "</index>\n";

            while (oldDestinations.entries() > 0 )
            {
               UtlString* pMailbox = (UtlString *) oldDestinations.removeAt(0);
               UtlString addr = pMailbox->data();
               delete pMailbox;

               distributionData += "    <destination>" + addr + "</destination>\n";
            }

            distributionData += "  </list>\n";
         }
      }   
      else
      {
         if ( m_action == "editdistribution" )
         {
            distributionData += "  <list>\n";
            distributionData += "    <index>" + indexName + "</index>\n";

            while (destinations.entries() > 0 )
            {
               UtlString* pMailbox = (UtlString *) destinations.removeAt(0);
               UtlString addr = pMailbox->data();
               delete pMailbox;

               distributionData += "    <destination>" + addr + "</destination>\n";
            }

            distributionData += "  </list>\n";
         }
      }
   }

   // Add the new list to the end of the file
   if ( m_action == "adddistribution" )
   {
      distributionData += "  <list>\n";
      distributionData += "    <index>" + m_index + "</index>\n";

      while (destinations.entries() > 0 )
      {
         UtlString* pMailbox = (UtlString *) destinations.removeAt(0);
         UtlString addr = pMailbox->data();
         delete pMailbox;

         distributionData += "    <destination>" + addr + "</destination>\n";
      }

      distributionData += "  </list>\n";
   }

   distributionData += "</distributions>\n";

   OsFile distributionFile ( m_filename );
   result = distributionFile.open( OsFile::CREATE );
   if ( result == OS_SUCCESS )
   {
      unsigned long bytes_written = 0;
      result = distributionFile.write( distributionData.data(), 
                                       distributionData.length(),
                                       bytes_written );
      distributionFile.close();
   }

   return result ;
}

OsStatus
ManageDistributionsWebCGI::validateAddress(const UtlString& address) const
{
   // Validate the mailbox id and extension.
   UtlString mailboxIdentity;
   UtlString extension;

   MailboxManager* pMailboxManager = MailboxManager::getInstance();
   OsStatus result = pMailboxManager->validateMailbox( address,
                                                       TRUE,
                                                       TRUE,
                                                       mailboxIdentity,
                                                       extension );

   return result;
}
