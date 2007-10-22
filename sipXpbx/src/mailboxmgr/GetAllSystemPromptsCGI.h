//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef GET_ALL_SYSTEM_PROMPTS_H
#define GET_ALL_SYSTEM_PROMPTS_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
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
 * GetAllSystemPromptsCGI Class
 *
 * Returns the VXML snippet for playing all the greetings of the mailbox owner.
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class GetAllSystemPromptsCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    GetAllSystemPromptsCGI(     const UtlBoolean& requestIsFromWebUI,
                            const UtlString& status,
                            const UtlString& promptType);

    /**
     * Virtual Destructor
     */
    virtual ~GetAllSystemPromptsCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out);

    /** Handles requests from the web and returns HTML output */
    OsStatus handleWebRequest( UtlString* out ) ;

    /** Handles requests from the IVR and returns VXML output */
        OsStatus handleOpenVXIRequest( UtlString* out ) ;



    /** Helper method to construct the VXML for playing all the system-wide greetings.
     *  There are 4 types of system greetings:
     *  (1) Standard system-wide greeting
     *  (2) After hours greeting
     *  (3) Special occasion greeting
     *  (4) Generic (pingtel supplied) greeting.
     *
     *  This method gets the URL of each of the above greetings and constructs the
     *  VXML snippet for playing them. If a particular greeting type has not been recorded,
     *  it is skipped.
     *
     *  @param  vxmlSnippet     Filled on return. VXML code for playing the WAVs.
     */
    OsStatus getSystemGreetings(UtlString& vxmlSnippet) ;



    /** Helper method to construct the VXML for playing all the autoattendant prompts.
     *  There are 2 types of auto attendant prompts:
     *  (1) User recorded prompt
     *  (2) Generic (pingtel supplied) prompt.
     *
     *  This method gets the URL of each of the above prompts and constructs the
     *  VXML snippet for playing them. If a particular prompt is not available,
     *  it is skipped.
     *
     *  @param  vxmlSnippet     Filled on return. VXML code for playing the WAVs.
     */
    OsStatus getAutoAttendantPrompts(UtlString& vxmlSnippet) ;



protected:

private:

    /** Flag indicating if this CGI was invoked from the web */
    const UtlBoolean m_fromWeb ;

    /** Used by web requests only. */
    const UtlString m_status ;

    /** Indicates the type of system prompt we need to handle.
     *
     *  This CGI can be used to retrieve both the system-wide greetings and autoattendant prompts.
     *  This string can two values:
     *  "greetings" -- CGI returns the URL of all the system-wide greetings.
     *  "autoattendant" -- CGI returns the URL of all the auto attendant prompts.
     */
    const UtlString m_promptType ;
};

#endif //GET_ALL_SYSTEM_PROMPTS_H
