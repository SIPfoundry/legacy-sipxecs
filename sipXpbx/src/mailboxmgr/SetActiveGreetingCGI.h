//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SETACTIVEGREETING_H
#define SETACTIVEGREETING_H

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
 * SetActiveGreetingCGI Class
 *
 * CGI for setting a greeting as the active greeting to be played to the callers
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class SetActiveGreetingCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    SetActiveGreetingCGI(       const UtlBoolean& requestIsFromWebUI,
                            const UtlString& mailboxIdentity,
                                                        const UtlString& greetingType ) ;

    /**
     * Virtual Destructor
     */
    virtual ~SetActiveGreetingCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

    OsStatus handleWebRequest( UtlString* out ) ;

        OsStatus handleOpenVXIRequest( UtlString* out ) ;


protected:

private:
    /** Fully qualified mailbox identity */
        UtlString m_mailboxIdentity;

    /** Type of greeting to be set as the active greeting */
        const UtlString m_greetingType;

    /** Flag indicating if the request was made from the web UI */
    const UtlBoolean m_fromWeb ;
};

#endif //SETACTIVEGREETING_H
