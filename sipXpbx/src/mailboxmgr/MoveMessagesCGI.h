//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef MOVEMESSAGESCGI_H
#define MOVEMESSAGESCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
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
 * MoveMessagesCGI Class
 *
 *      CGI for moving messages from one folder to another.
 *      For example, when messages are saved, they will be moved from inbox to saved.
 *      When messages are deleted, they will be moved from inbox or saved to deleted folder.
 *
 * @author Harippriya
 * @version 1.0
 */
class MoveMessagesCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    MoveMessagesCGI(const UtlBoolean& requestIsFromWebUI,
                    const UtlString& mailbox,
                                        const UtlString& fromFolder,
                                        const UtlString& toFolder,
                                        const UtlString& messageIds,
                                        const UtlString& maintainstatus,
                    const UtlString& nextblockhandle) ;

    /**
     * Virtual Dtor
     */
    virtual ~MoveMessagesCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

    OsStatus handleWebRequest( UtlString* out ) ;

        OsStatus handleOpenVXIRequest( UtlString* out ) ;

protected:

private:
        /** Fully qualified mailbox id of the user. For eg. sip:hari@pingtel.com */
    UtlString m_mailboxIdentity;

        /** Current location of the message */
        const UtlString m_fromFolder;

        /** Destination of the message */
        const UtlString m_toFolder;

        /** List of message ids separated by a space */
        const UtlString m_messageIds;

        /** Flag indicating whether the unheard state of the message should be
         *      maintained after the move.
         *      Set to 'yes' for TUI requests.
         *  Set to 'no' for Web UI requests.
         */
        const UtlString m_maintainstatus;

    const UtlBoolean m_fromWeb;

    /** This variable is used to make sure that at the end of the edit,
        the right webpage is displayed (if there are more than one webpages used
        to display the contents of the single folder)
    */
    const UtlString m_nextBlockHandle ;
};

#endif //MOVEMESSAGESCGI_H
