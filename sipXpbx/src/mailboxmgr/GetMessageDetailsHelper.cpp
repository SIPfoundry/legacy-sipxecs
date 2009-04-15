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
#include "utl/UtlHashMap.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/GetMessageDetailsHelper.h"
#include "mailboxmgr/MailboxManager.h"




// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

GetMessageDetailsHelper::GetMessageDetailsHelper()
{}

GetMessageDetailsHelper::~GetMessageDetailsHelper()
{}

OsStatus
GetMessageDetailsHelper::execute(UtlString* out)
{
   return OS_SUCCESS;
}


OsStatus
GetMessageDetailsHelper::getMessageBlock(
   const UtlString& mailbox,
   const UtlString& category,
   const int blocksize,
   const int nextBlockHandle,
   UtlBoolean& endOfMessages,
   const UtlBoolean& isFromWeb,
   const UtlBoolean& msgOrderDescending )
{
   OsStatus result = OS_SUCCESS;

   // Instantiate the mailbox manager
   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   // Call the method on Mailbox Manager to do the actual work.

   // Mailbox manager populates UtlDList object - m_messageblock.
   // m_messageblock is a collection UtlHashMap objects -
   // one for each message.
   result = pMailboxManager->getMessageBlock (
      mailbox,
      m_messageblock,
      category,
      blocksize,
      nextBlockHandle,
      endOfMessages,
      isFromWeb,
      msgOrderDescending);

   return result;
}


OsStatus
GetMessageDetailsHelper::getNextMessageCollectable()
{
   // Call the get() method on m_messageblock.
   // This will retrieve the first UtlHashMap object from its list.
   // Saves this object in m_messagedetails.

   OsStatus result;

   if ( m_messageblock.isEmpty() )
   {
      result = OS_FAILED ;

   } else {
      m_messageCollectable = (UtlDList*) m_messageblock.get();
      result = OS_SUCCESS;
   }

   timestamp = "";

   /* This is a better way of doing the task.
      TODO : Fix the compile issues.
      OsStatus result = OS_FAILED;

      if( m_messageblock != NULL && !m_messageblock.isEmpty() )
      {
      m_messagedetails = (UtlHashMap*) m_messageblock.get();
      result = OS_SUCCESS;
      }
   */

   return result;
}


OsStatus
GetMessageDetailsHelper::getNextMessage()
{
   // Call the get() method on m_messageblock.
   // This will retrieve the first UtlHashMap object from its list.
   // Saves this object in m_messagedetails.

   OsStatus result;

   if( m_messageCollectable == NULL )
   {
      result = OS_FAILED;

   } else if ( m_messageCollectable->isEmpty() )
   {
      result = OS_FAILED;

   } else {
      m_messagedetails = (UtlHashMap*) m_messageCollectable->get();
      result = OS_SUCCESS;
   }


   /* This is a better way of doing the task.
      TODO : Fix the compile issues.
      OsStatus result = OS_FAILED;

      if( m_messageblock != NULL && !m_messageblock.isEmpty() )
      {
      m_messagedetails = (UtlHashMap*) m_messageblock.get();
      result = OS_SUCCESS;
      }
   */

   return result;
}

UtlString
GetMessageDetailsHelper::getMessageDetails(const UtlString& key) const
{
   // Parse the UtlHashMap object corresponding to a single message
   // (stored in m_messagedetails) to get the value.

   UtlString retValue;

   if ( m_messagedetails == NULL )
   {
      retValue = "";
   }
   else
   {
      UtlString* collectableKey = new UtlString(key);
      UtlString* value =
         (UtlString*) m_messagedetails->findValue( collectableKey );
      retValue = value->data();
      delete collectableKey;
      delete value;
   }

   return retValue;
}

OsStatus
GetMessageDetailsHelper::getMessageAttachmentsCount(int& count) const
{
   if( m_messageCollectable == NULL )
      count = 1;
   else if ( m_messageCollectable->isEmpty() )
      count = 1;
   else
      count = m_messageCollectable->entries();

   return OS_SUCCESS;
}

UtlString
GetMessageDetailsHelper::getMediaserverUrl() const
{
   UtlString mediaserverUrl;

   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   // Call the method on Mailbox Manager to do the actual work.
   pMailboxManager->getMediaserverURL( mediaserverUrl );

   return mediaserverUrl;
}


UtlString
GetMessageDetailsHelper::getIvrPromptUrl() const
{
   UtlString ivrPromptUrl;

   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   // Call the method on Mailbox Manager to do the actual work.
   pMailboxManager->getIvrPromptURL( ivrPromptUrl );

   return ivrPromptUrl;
}


UtlString
GetMessageDetailsHelper::getMediaserverSecureUrl() const
{
   UtlString mediaserverSecureUrl;

   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   // Call the method on Mailbox Manager to do the actual work.
   pMailboxManager->getMediaserverSecureURL( mediaserverSecureUrl );

   return mediaserverSecureUrl;
}


UtlBoolean
GetMessageDetailsHelper::enableVoicemailInfoPlayback() const
{
   // Set to FALSE to DISABLE voicemail info playback by default
   UtlBoolean enableInfoPlayback = FALSE ;
   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   UtlString paramValue;
   pMailboxManager->getCustomParameter( PARAM_VOICEMAIL_INFO_PLAYBACK, paramValue );

   if( !paramValue.isNull() )
   {
      if( paramValue.compareTo("ENABLE", UtlString::ignoreCase) == 0 )
         enableInfoPlayback = TRUE;
   }

   return enableInfoPlayback;
}
