// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef TRANSFER_TO_EXTN_CGI_H
#define TRANSFER_TO_EXTN_CGI_H

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
 *  TransferToExtnCGI Class
 *
 *  CGI for taking the extension dialed by the user and converting it 
 *  into a fully qualified SIP URL and then transfering to that extension.
 *
 *  @author Harippriya M Sivapatham
 *  @version 1.0
 */
class TransferToExtnCGI : public VXMLCGICommand
{
public:
    /**
     * Ctor
     */
    TransferToExtnCGI ( const UtlString& extension );

    /**
     * Virtual Dtor
     */
    virtual ~TransferToExtnCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

protected:

private:
    const UtlString m_extension;
};

#endif //TRANSFER_TO_EXTN_CGI_H

