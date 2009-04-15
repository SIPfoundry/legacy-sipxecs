// 
// 
// Copyright (C) 2007 VoiceWorks Sp. z o.o.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef VXMLHELPER_H
#define VXMLHELPER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "utl/UtlString.h"
#include "CGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS


// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 *  VXMLCGICommand
 *
 *  Provides methods for constructing VXML document header with language attribute
 *
 *  @author Pawel Pierscionek
 *  @version 1.0
 */

class VXMLCGICommand: public CGICommand
{
private:
    UtlString m_lang;

    /** Returns the type of the active greeting
     *
     *  @return UtlString       VXML languate attribute with value
     */
    UtlString getLangAttr();

public:
    /**
     * Ctor
     */
    VXMLCGICommand();

    /**
     * Destructor
     */
    virtual ~VXMLCGICommand() {};

    /** Sets language of VXML document
     *
     *  @param lang  		Language (eg. en-US)
     *
     *  @return
     */
    void setLang(UtlString lang);

    /** Returns VXML document header 
     *
     *  @param application  	Optional application to be included in the VXML header
     *
     *  @return UtlString	VXML document header
     */
    UtlString getVXMLHeader(UtlString application = NULL);

protected:

};

#endif //VXMLHELPER_H

