// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SAVEMESSAGE_H
#define SAVEMESSAGE_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "net/Url.h"
#include "mailboxmgr/VXMLCGICommand.h"
#include "mailboxmgr/MailboxManager.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * SaveMessage Class
 *
 * @author John P. Coffey
 * @version 1.0
 */
class SaveMessage : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    SaveMessage ( const Url& from, 
                  const UtlString& identityOrExtension, 
                  const UtlString& durationsecs,
                  const UtlString& timestamp,
                  const char* termchar, 
                  const char* data, 
                  int   datasize );
    /**
     * Virtual Destructor
     */
    virtual ~SaveMessage();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL) ;

protected:

private:
    const Url m_from;
    const UtlString m_identityOrExtension;
    const UtlString m_duration;
    const UtlString m_timestamp;
    UtlString m_termchar;
    char* m_data;
    int   m_datasize;
};

#endif //SAVEMESSAGE_H
