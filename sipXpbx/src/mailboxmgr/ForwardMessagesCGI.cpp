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
#include "mailboxmgr/ForwardMessagesCGI.h"
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
ForwardMessagesCGI::ForwardMessagesCGI(
    const char* comments,
    const UtlString& commentsDuration,
    const UtlString& commentsTimestamp,
    const int commentsSize,
    const Url& fromMailbox,
    const UtlString& fromFolder,
    const UtlString& messageIds,
    const UtlString& toExtension  ) :
        m_commentsDuration ( commentsDuration ),
    m_commentsTimestamp ( commentsTimestamp ),
    m_fromMailboxIdentity ( fromMailbox ),
        m_fromFolder ( fromFolder ),
        m_messageIds ( messageIds ),
        m_toExtension ( toExtension ),
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

ForwardMessagesCGI::~ForwardMessagesCGI()
{
  if (m_comments && m_commentsSize > 0)
    delete[] m_comments;

}

OsStatus
ForwardMessagesCGI::execute(UtlString* out)
{
        OsStatus result = OS_SUCCESS;
        UtlString dynamicVxml = getVXMLHeader();

        ValidateMailboxCGIHelper validateMailboxHelper ( m_toExtension );

    // Lazy create and validate the designated mailbox
    result = validateMailboxHelper.execute( out );

    if ( result == OS_SUCCESS )
    {
                // extension or the mailbox id is valid.
                UtlString toMailboxIdentity;
                validateMailboxHelper.getMailboxIdentity( toMailboxIdentity );

                MailboxManager* pMailboxManager =
                        MailboxManager::getInstance();

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


                if( result == OS_SUCCESS )
                {
                        // Message was forwarded successfully
                        dynamicVxml += VXML_SUCCESS_SNIPPET;
                }
                else
                {
                        // Unable to forward the message
                        dynamicVxml += VXML_FAILURE_SNIPPET;
                }
        }
        else
        {
                // Invalid extension or mailbox id.
                dynamicVxml += VXML_INVALID_EXTN_SNIPPET ;
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
