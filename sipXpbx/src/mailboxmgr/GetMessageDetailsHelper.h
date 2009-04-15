//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef GET_MESSAGE_DETAILS_HELPER_H
#define GET_MESSAGE_DETAILS_HELPER_H

// SYSTEM INCLUDES
//#include <...>



// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "utl/UtlDList.h"

#include "mailboxmgr/VXMLCGICommand.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlHashMap;

/**
 * Helper for the CGI that is used to play messages.
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class GetMessageDetailsHelper : public VXMLCGICommand
{
  public:
   /**
    * Ctor
    */
   GetMessageDetailsHelper ();

   /**
    * Virtual Dtor
    */
   virtual ~GetMessageDetailsHelper();

   /** This does the work */
   virtual OsStatus execute (UtlString* out = NULL);


   OsStatus getMessageBlock (
      const UtlString& mailbox,
      const UtlString& category,
      const int blocksize,
      const int nextBlockHandle,
      UtlBoolean& endOfMessages,
      const UtlBoolean& isFromWeb,
      const UtlBoolean& msgOrderDescending = FALSE);

   OsStatus getNextMessage();

   OsStatus getNextMessageCollectable();

   UtlString getMessageDetails(const UtlString& key) const;

   OsStatus getMessageAttachmentsCount(int& count) const;

   UtlString getIvrPromptUrl() const;

   UtlString getMediaserverUrl() const;

   UtlString getMediaserverSecureUrl() const;

   /**  Helper method for determining if the message envelope needs to be played
    *   before playing the message. Invokes the MailboxManager method for querying
    *   the value of <voicemail-info-playback> element in voicemail.xml file
    *   and returns a boolean based on the value of the element.
    *
    *   @return UtlBoolean      TRUE if the element's value is ENABLE (case-insensitive)
    *                           FALSE for all other values
    */
   UtlBoolean enableVoicemailInfoPlayback() const;


  protected:

  private:
   UtlDList m_messageblock;

   UtlDList* m_messageCollectable;

   UtlHashMap*  m_messagedetails;

   UtlString timestamp;
};

#endif //GET_MESSAGE_DETAILS_HELPER_H
