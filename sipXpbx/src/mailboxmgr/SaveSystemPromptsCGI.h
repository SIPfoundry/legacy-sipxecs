//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SAVESYSTEMPROMPTS_H
#define SAVESYSTEMPROMPTS_H

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
 * Saves the system prompt and makes it active.
 * There are two types of system prompts -- (1) System-wide greeting (2) Autoattendant prompt.
 *
 * System-wide Greetings:
 * The system-wide greeting is played when a caller dials the organization's main number
 * or dials the auto attendant. The generic system-wide greeting that is shipped
 * with SIPxchange is "Welcome to Pingtel Communications System".
 * Administrators can record three types of system greetings - standard, after hours and
 * special occasion.
 *
 * Auto attendant prompt:
 * Auto attendant prompt is played after the system greetings when
 * a caller dials the organization's main number or dials the auto attendant.
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class SaveSystemPromptsCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    SaveSystemPromptsCGI(   const UtlString& promptType,
                                                const char* data,
                                                int   datasize);

    /**
     * Virtual Destructor
     */
    virtual ~SaveSystemPromptsCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out);

protected:

private:

        /** Type of system-wide prompt to be saved.
         *      Possible values: standard, afterhours, special, autoattendant
         *      Used to save the recorded data in appropriate filenames.
         */
        UtlString m_promptType;

        /**     Recorded data */
        char* m_data ;

        /**     Size of recorded data*/
        int m_datasize;

};

#endif //SAVESYSTEMPROMPTS_H
