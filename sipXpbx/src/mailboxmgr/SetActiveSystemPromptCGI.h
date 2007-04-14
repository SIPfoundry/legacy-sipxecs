//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SETACTIVESYSTEMPROMPT_H
#define SETACTIVESYSTEMPROMPT_H

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
 * SetActiveSystemPromptCGI Class
 *
 * CGI for setting a greeting as the active greeting to be played to the callers
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class SetActiveSystemPromptCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    SetActiveSystemPromptCGI(   const UtlBoolean& requestIsFromWebUI,
                                const UtlString& promptType ) ;

    /**
     * Virtual Destructor
     */
    virtual ~SetActiveSystemPromptCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

    /** Handles requests from the web and returns the HTML output */
    OsStatus handleWebRequest( UtlString* out ) ;

    /** Handles requests from the IVR and returns the VXML output */
        OsStatus handleOpenVXIRequest( UtlString* out ) ;


protected:

private:

    /** Type of prompt to be set as active */
        const UtlString m_promptType;

    /** Flag indicating if the request was made from the web UI */
    const UtlBoolean m_fromWeb ;
};

#endif //SETACTIVESYSTEMPROMPT_H
