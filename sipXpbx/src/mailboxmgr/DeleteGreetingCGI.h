//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef DeleteGreetingCGI_H
#define DeleteGreetingCGI_H

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
 *  DeleteGreetingCGI Class
 *
 *  CGI for deleting a user's greeting.
 *  This CGI merely calls the appropriate method in the MailboxManager class
 *  for the actual delete.
 *  Currently, it is used only by the WebUI.
 *
 *  @author Harippriya M Sivapatham
 *  @version 1.0
 */
class DeleteGreetingCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    DeleteGreetingCGI(  const UtlBoolean& requestIsFromWebUI,
                        const UtlString& mailbox,
                                                const UtlString& greetingType,
                        const UtlBoolean& isActiveGreeting) ;

    /**
     * Virtual Destructor
     */
    virtual ~DeleteGreetingCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

    OsStatus handleWebRequest( UtlString* out ) ;

        OsStatus handleOpenVXIRequest( UtlString* out ) ;


protected:

private:

    /** Fully qualified mailbox id */
        UtlString m_mailboxIdentity;

    /** Type of greeting to be deleted.
        Possible values are: "standard", "outofoffice", "extendedabsence", "name"
    */
        const UtlString m_greetingType;

    /** Flag indicating if the request was made from the web UI */
    const UtlBoolean m_fromWeb ;

    /** Flag indicating if the greeting to be deleted is the active greeting.
        If it is, then additional steps have to be done to reset the active greeting setting.
    */
    const UtlBoolean m_isActiveGreeting ;
};

#endif //DeleteGreetingCGI_H
