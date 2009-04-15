// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SOSCGI_H
#define SOSCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "net/Url.h"
#include "mailboxmgr/VXMLCGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SosCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    SosCGI (const Url& from);

    /**
     * Virtual Dtor
     */
    virtual ~SosCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

    OsStatus parseMappingFile(const UtlString& mapFile);

protected:

private:
    UtlString mFrom;
    UtlString mSosUrl;
};

#endif //SOSCGI_H

