//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _CpTestSupport_h_
#define _CpTestSupport_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CallManager;
class SipUserAgent;

class CpTestSupport
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /**
     * A testable call manager initialized to testable defaults
     */
    static CallManager *newCallManager(SipUserAgent *ua);

    /**
     * A testable user agent initialized to testable defaults
     */
    static SipUserAgent *newSipUserAgent();

};

#endif  // _CpTestSupport_h_
