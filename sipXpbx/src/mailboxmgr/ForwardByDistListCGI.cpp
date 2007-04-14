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
#include "mailboxmgr/ForwardByDistListCGI.h"
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
ForwardByDistListCGI::ForwardByDistListCGI(	
    const char* comments,
    const UtlString& commentsDuration,
    const UtlString& commentsTimestamp,
    const int commentsSize,
    const Url& fromMailbox,
    const UtlString& fromFolder, 
    const UtlString& messageIds,
    const UtlString& toDistList  ) :
   m_commentsDuration ( commentsDuration ),
   m_commentsTimestamp ( commentsTimestamp ),
   m_fromMailboxIdentity ( fromMailbox ),
   m_fromFolder ( fromFolder ),
   m_messageIds ( messageIds ),
   m_toDistList ( toDistList ),
   m_commentsSize ( commentsSize )
{
   if (comments && commentsSize > 0)
   {
      m_comments = new char[commentsSize + 1];
      if (m_comments)
      {
         memcpy(m_comments, comments, commentsSize);
         m_comments[commentsSize] = 0;
      }
   }
}

ForwardByDistListCGI::~ForwardByDistListCGI()
{
   if (m_comments && m_commentsSize > 0)
   {
      delete[] m_comments;
   }
}

OsStatus
ForwardByDistListCGI::execute(UtlString* out) 
{
   OsStatus result = OS_SUCCESS;
   bool atLeastOneSucceed = false;
   UtlString dynamicVxml (VXML_BODY_BEGIN);

   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   // Get the distribution list file
   UtlString mailboxPath;
   UtlString mailboxIdentity;

   m_fromMailboxIdentity.getIdentity(mailboxIdentity);
   result = pMailboxManager->getMailboxPath(mailboxIdentity, mailboxPath);

   if( result == OS_SUCCESS )
   {
      UtlString filename = mailboxPath + OsPathBase::separator + DIST_LIST_NAME;

      // Parse the distribution list file
      UtlSortedList destinations;
      result = pMailboxManager->parseDistributionFile (filename, m_toDistList, destinations);

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
               UtlString toMailboxIdentity;
               validateMailboxHelper.getMailboxIdentity( toMailboxIdentity );


               // Forward call to Mailbox Manager where the configuration 
               // header subject etc can be added to the filename
               result = pMailboxManager->forwardMessages(
                  m_comments, 
                  m_commentsDuration, 
                  m_commentsTimestamp,
                  m_commentsSize, 
                  m_fromMailboxIdentity, 
                  m_fromFolder,
                  m_messageIds, 
                  toMailboxIdentity );

               if (result == OS_SUCCESS && !atLeastOneSucceed)
               {
                  atLeastOneSucceed = true;
               }
            }
         }

         if(atLeastOneSucceed)
         {
            // Message was forwarded successfully
            dynamicVxml += VXML_SUCCESS_SNIPPET;
         }
         else
         {
            // We only complain it if nothing has been forwarded
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
