//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef RECYCLEDELETEDMSGSCGI_H
#define RECYCLEDELETEDMSGSCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mailboxmgr/VXMLCGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * RecycleDeletedMsgsCGI Class
 *
 *      CGI for deleting messages in the 'Deleted' folder.
 *
 * @author Harippriya
 * @version 1.0
 */
class RecycleDeletedMsgsCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    RecycleDeletedMsgsCGI(  const UtlBoolean& requestIsFromWebUI,
                            const UtlString& m_mailboxIdentity,
                            const UtlString& messageIds,
                            const UtlString& nextblockhandle) ;

    /**
     * Virtual Dtor
     */
    virtual ~RecycleDeletedMsgsCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

    OsStatus handleWebRequest( UtlString* out );

        OsStatus handleOpenVXIRequest( UtlString* out );

protected:

private:

        /** Fully qualified mailbox id of the user. For eg. sip:hari@pingtel.com */
    UtlString m_mailboxIdentity;

    /** Flag indicating if this CGI was called from the User UI */
    const UtlBoolean m_fromWeb ;

    // List of space separated message ids that need to be purged.
    UtlString m_messageids ;

    /** This variable is used to make sure that at the end of the edit,
        the right webpage is displayed (if there are more than one webpages used
        to display the contents of the single folder)
    */
    const UtlString m_nextBlockHandle ;
};

#endif //RECYCLEDELETEDMSGSCGI_H
