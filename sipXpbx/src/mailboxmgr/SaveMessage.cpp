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
#include "mailboxmgr/SaveMessage.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */
SaveMessage::SaveMessage( const Url& from,
                          const UtlString& identityOrExtension,
                          const UtlString& duration,
                          const UtlString& timestamp,
                          const char* termchar,
                          const char* data,
                          int   datasize ) :
    m_from ( from ),
    m_identityOrExtension ( identityOrExtension ),
    m_duration ( duration ),
    m_timestamp ( timestamp ),
    m_termchar( "" ),
    m_datasize ( datasize )
{
    if (termchar)
       m_termchar = termchar;

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

SaveMessage::~SaveMessage()
{
  if (m_data)
    delete[] m_data;

}

OsStatus
SaveMessage::execute(UtlString* out)
{
    OsStatus result = OS_SUCCESS;
        UtlString dynamicVxml (VXML_BODY_BEGIN);

    MailboxManager* pMailboxManager =
            MailboxManager::getInstance();
    int minMessageLength;
    pMailboxManager->getMinMessageLength( minMessageLength ) ;

    int recordedMsgLength = atoi(m_duration.data()) ;

   if (!m_duration.isNull() && recordedMsgLength > minMessageLength)
   {
           ValidateMailboxCGIHelper validateMailboxHelper ( m_identityOrExtension );

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

                   if( result == OS_SUCCESS )
                   {
                           // Message was saved successfully
                           dynamicVxml += VXML_SUCCESS_SNIPPET;
           } else
                   {
                           // Unable to save the message
                           dynamicVxml += VXML_FAILURE_SNIPPET;
                   }
       } else
           {
                   // Invalid extension or mailbox id.
                   dynamicVxml += VXML_INVALID_EXTN_SNIPPET ;
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
