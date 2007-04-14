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
#include "os/OsFS.h"
#include "utl/UtlString.h"
#include "mailboxmgr/VXMLDefs.h"
#include "cbadmissioncgi/ConferenceBridgeCGI.h"
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

ConferenceBridgeCGI::ConferenceBridgeCGI(const UtlString& name,
                                         const UtlString& contact,
                                         const UtlString& confId,
                                         const UtlString& accessCode) :
   mContact(contact),
   mName(name),
   mConfId(confId),
   mAccessCode(accessCode)
{
}

ConferenceBridgeCGI::~ConferenceBridgeCGI()
{
}

OsStatus
ConferenceBridgeCGI::execute(UtlString* out)
{
    OsStatus result = OS_FAILED;

    // Get the base URL of mediaserver. This is necessary for playing prompts.
    ConferenceManager* pConferenceManager = ConferenceManager::getInstance();

    UtlString ivrPromptUrl;
    pConferenceManager->getIvrPromptURL( ivrPromptUrl );

    UtlString mediaserverUrl;
    ConferenceManager::getInstance()-> getMediaserverURL( mediaserverUrl );

    UtlString secureMediaserverUrl;
    pConferenceManager->getMediaserverSecureURL( secureMediaserverUrl );
    
    // For testing purpose
    mContact = "sip:dong@pingtel.com";
    
    // Construct the dynamic VXML
    UtlString dynamicVxml(VXML_BODY_BEGIN);

    // Validate the access code
    if (!mAccessCode.isNull())
    {
        UtlString conferenceUrl;
        result = pConferenceManager->doLogin(mContact, mConfId, mAccessCode, conferenceUrl);

        if ( result == OS_SUCCESS )
        {
            // Contains the dynamically generated VXML script.
            dynamicVxml += "<form> \n" \
                               "<transfer dest=\"" + conferenceUrl + "\" /> \n" \
                           "</form> \n";
        }
        else
        {
           dynamicVxml += VXML_INVALID_EXTN_SNIPPET;
        }
    }
    else
    {
         
        dynamicVxml += "<form>\n";
        dynamicVxml += "<subdialog name=\"conferencebridge\" src=\"" + mediaserverUrl + "/cb_vxml/" + mName + ".vxml\">\n";
        dynamicVxml += "<param name=\"contact\" expr=\"'" + mContact + "'\"/>\n" \
                       "<param name=\"confid\" expr=\"'" + mConfId + "'\"/>\n" \
                       "<param name=\"mediaserverurl\" expr=\"'" + ivrPromptUrl + "'\"/>\n" \
                       "<param name=\"securemediaserverurl\" expr=\"'" + secureMediaserverUrl + "'\"/>\n" \
                       "</subdialog>\n" \
                       "</form>\n";
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
    return OS_SUCCESS;
}


