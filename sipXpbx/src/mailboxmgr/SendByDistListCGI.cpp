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
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"
#include "mailboxmgr/SendByDistListCGI.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */
SendByDistListCGI::SendByDistListCGI( const Url& from,
                                      const UtlString& identity,
                                      const UtlString& distlist, 
                                      const UtlString& duration,
                                      const UtlString& timestamp,
                                      const char* termchar, 
                                      const char* data, 
                                      int   datasize ) :
    m_from ( from ),
    m_identity ( identity ),
    m_distlist ( distlist ),
    m_duration ( duration ),
    m_timestamp ( timestamp ),
    m_termchar( "" ),
    m_datasize ( datasize )
{
   if (termchar)
   {
      m_termchar = termchar;
   }

   if (data && datasize > 0)
   {
      m_data = new char[datasize + 1];
      if (m_data)
      {
         memcpy(m_data, data, datasize);
         m_data[datasize] = 0;
      }
   }
}

SendByDistListCGI::~SendByDistListCGI()
{
   if (m_data)
   {
      delete[] m_data;
   }
}

OsStatus
SendByDistListCGI::execute(UtlString* out) 
{
   OsStatus result = OS_SUCCESS;
   bool atLeastOneSucceed = false;
   
   UtlString dynamicVxml = getVXMLHeader();

   MailboxManager* pMailboxManager = MailboxManager::getInstance();
   int minMessageLength;
   pMailboxManager->getMinMessageLength( minMessageLength ) ;

   int recordedMsgLength = atoi(m_duration.data()) ;

   if (!m_duration.isNull() && recordedMsgLength > minMessageLength)
   {
      // Get the distribution list file
      UtlString mailboxPath;
      result = pMailboxManager->getMailboxPath(m_identity, mailboxPath);

      if( result == OS_SUCCESS )
      {
         UtlString filename = mailboxPath + OsPathBase::separator + DIST_LIST_NAME;

         // Parse the distribution list file
         UtlSortedList destinations;
         result = pMailboxManager->parseDistributionFile (filename, m_distlist, destinations);

         if( result == OS_SUCCESS )
         {
            // Loop through the distribution list
            while (destinations.entries() > 0 )
            {
               UtlString* pMailbox = (UtlString *) destinations.removeAt(0);
               UtlString mailbox = pMailbox->data();
               delete pMailbox;

               ValidateMailboxCGIHelper validateMailboxHelper ( mailbox );

               // Lazy create and validate the designated mailbox
               result = validateMailboxHelper.execute( out );

               if ( result == OS_SUCCESS ) 
               {
                  // extension or the mailbox id is valid.
                  UtlString mailboxIdentity;
                  validateMailboxHelper.getMailboxIdentity( mailboxIdentity );


                  // Forward call to Mailbox Manager where the configuration 
                  // header subject etc can be added to the filename
                  result = pMailboxManager->saveMessage(
                     m_from, 
                     mailboxIdentity, 
                     m_duration,
                     m_timestamp, 
                     m_data, 
                     m_datasize );
                     
                  if (result == OS_SUCCESS && !atLeastOneSucceed)
                  {
                     atLeastOneSucceed = true;
                  }
               }
            }

            if(atLeastOneSucceed)
            {
               // Message was saved successfully
               dynamicVxml += VXML_SUCCESS_SNIPPET;
            }
            else
            {
               // We only complain it if nothing has been saved
               dynamicVxml += VXML_FAILURE_SNIPPET;
            }
         }
         else
         {
            // Invalid distribution list
            dynamicVxml += VXML_INVALID_LIST_SNIPPET ;
         }
      }
      else
      {
         // Unable to get the mailbox path
         dynamicVxml += VXML_FAILURE_SNIPPET;
      }
   }
   else
   {
      // Message too short
      dynamicVxml += "<form> <block>\n" \
         "<var name=\"result\" expr=\"'msgtooshort'\"/>\n" \
         "<return namelist=\"result\"/>\n"
         "</block> </form>\n";
   }
   
   dynamicVxml += VXML_END;

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
