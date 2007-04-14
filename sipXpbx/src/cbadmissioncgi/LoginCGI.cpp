//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "LoginCGI.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/HTMLDefs.h"
#include "cbadmissioncgi/ConferenceManager.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

LoginCGI::LoginCGI(const UtlString& contact,
                   const UtlString& confId,
                   const UtlString& accessCode) :
        mContact( contact ),
        mConfId ( confId ),
        mAccessCode ( accessCode )
{}

LoginCGI::~LoginCGI()
{}

OsStatus
LoginCGI::execute(UtlString* out)
{
    OsStatus result = OS_FAILED;

    // Instantiate the mailbox manager
    ConferenceManager* pConferenceManager = ConferenceManager::getInstance();

    UtlString dynamicVxml (VXML_BODY_BEGIN);

    // Validate the access code
    UtlString conferenceUrl;
    result = pConferenceManager->doLogin(mContact, mConfId, mAccessCode, conferenceUrl);

    if ( result == OS_SUCCESS )
    {
        dynamicVxml += "<form>\n<block>\n";
        dynamicVxml += "<var name=\"result\" expr=\"'success'\"/>\n";
        dynamicVxml += "<var name=\"conferenceurl\" expr=\"'" + conferenceUrl + "'\"/>\n";
        dynamicVxml += "<return namelist=\"result conferenceurl\"/>\n";
        dynamicVxml += "</block>\n</form>\n";
    }
    else
    {
        dynamicVxml += VXML_FAILURE_SNIPPET;
    }

    dynamicVxml += VXML_END;

    // Write out the dynamic VXML script to be processed by OpenVXI
    if (out)
    {
        out->remove(0);
        UtlString responseHeaders;
        ConferenceManager::getResponseHeaders(dynamicVxml.length(), responseHeaders);

        out->append(responseHeaders.data());
        out->append(dynamicVxml.data());
        }

        return OS_SUCCESS ;
}
