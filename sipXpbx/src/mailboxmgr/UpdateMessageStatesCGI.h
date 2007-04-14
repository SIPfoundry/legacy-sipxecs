//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef UPDATEMESSAGESTATES_H
#define UPDATEMESSAGESTATES_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "mailboxmgr/CGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UpdateMessageStatesCGI Class
 *
 * Changes the state of the messages from unheard to heard by deleting the
 * .sta files corresponding the messages.
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class UpdateMessageStatesCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    UpdateMessageStatesCGI(     const UtlBoolean& requestIsFromWebUI,
                            const UtlBoolean& linkInEmail,
                            const UtlString& mailboxIdentity,
                                                        const UtlString& category,
                                                        const UtlString& messageIds) ;

    /**
     * Virtual Destructor
     */
    virtual ~UpdateMessageStatesCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

    OsStatus handleWebRequest( UtlString* out );

    OsStatus handleEmailRequest( UtlString* out );

        OsStatus handleOpenVXIRequest( UtlString* out );


protected:

private:

        /** Fully qualified mailbox id of the user. For eg. sip:hari@pingtel.com */
        UtlString m_mailboxIdentity;

        /** Indicates the folder in which the messages to be updated are available.
         *      It can take the following values:
         *      1. unheard, heard or inbox -- messages in the inbox folder.
         *      2. saved   -- messages in the saved folder.
         *      3. custom folder name -- messages in that custom folder.
         */
        UtlString m_category;

        /** Message ids whose status has to be updated.
         *      It can values in two formats:
         *      1.      List of IDs separated by a space.
         *              For eg. "00000001 00000002" (used by the WebUI)
         *
         *      2.      "-1" to indicate that all messages in that folder have to be updated.
         *              Used on the TUI.
         */
        UtlString m_messageIds;

    /** Flag indicating if this CGI was called from the User UI */
    const UtlBoolean m_fromWeb ;

    /** Flag to indicate if the CGI was accessed from link provided in the
     *  voicemail notification email.
     */
    const UtlBoolean m_linkInEmail ;

};

#endif //UPDATEMESSAGESTATES_H
