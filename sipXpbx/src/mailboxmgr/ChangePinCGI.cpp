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
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"
#include "mailboxmgr/ChangePinCGI.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

ChangePinCGI::ChangePinCGI( const UtlString& userid,
                            const UtlString& password,
                            const UtlString& newPassword ) :
   mUserId ( userid ),
   mOldPassword ( password ),
   mNewPassword ( newPassword )
{}

ChangePinCGI::~ChangePinCGI()
{}

OsStatus
ChangePinCGI::execute(UtlString* out)
{
   OsStatus result = OS_SUCCESS;

   // Instantiate the mailbox manager
   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   UtlString dynamicVxml (VXML_BODY_BEGIN);

   // Validate the old password
   UtlString mailboxIdentity;
   UtlString extension;
   result = pMailboxManager->doLogin( mUserId,           // this is an extension
                                      mOldPassword,      // this is actually a PIN
                                      mailboxIdentity,   // filld in on return
                                      extension );       // filled in on return

   if ( result == OS_SUCCESS )
   {
      // Call the method on Mailbox Manager to set the new password
      result = pMailboxManager->setPassword( mailboxIdentity, mNewPassword );

      if( result == OS_SUCCESS )
      {
         // Password was saved successfully
         dynamicVxml += VXML_SUCCESS_SNIPPET;
      }
      else
      {
         // Unable to save the password
         dynamicVxml += VXML_FAILURE_SNIPPET;
      }
   }
   else
   {
      // Invalid distribution list
      dynamicVxml += VXML_INVALID_PIN_SNIPPET ;
   }

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

   return OS_SUCCESS ;
}
