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
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/GetAllSystemPromptsCGI.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

GetAllSystemPromptsCGI::GetAllSystemPromptsCGI( const UtlBoolean& requestIsFromWebUI,
                                                const UtlString& status,
                                                const UtlString& promptType ) :
    m_fromWeb ( requestIsFromWebUI ),
    m_status ( status ),
    m_promptType ( promptType )
{}

GetAllSystemPromptsCGI::~GetAllSystemPromptsCGI()
{}

OsStatus
GetAllSystemPromptsCGI::execute(UtlString* out)
{
    OsStatus result;
        if( m_fromWeb )
                result = handleWebRequest( out ) ;
        else
                result = handleOpenVXIRequest( out ) ;

    return result;
}

OsStatus
GetAllSystemPromptsCGI::handleOpenVXIRequest(UtlString* out)
{
    OsStatus result = OS_SUCCESS ;
        UtlString dynamicVxml (VXML_BODY_BEGIN);

    // Get the VXML for playing all the available prompts for the given prompt type.
    UtlString vxmlSnippet;
    if( m_promptType == "greetings" )
        result = getSystemGreetings( vxmlSnippet );
    else
        result = getAutoAttendantPrompts( vxmlSnippet );

    if( result == OS_SUCCESS )
    {
        dynamicVxml +=  "<form>\n" \
                                                "<block>\n" + vxmlSnippet + "</block>\n" \
                        "</form>\n" ;
    }
    else
    {
        dynamicVxml += VXML_FAILURE_SNIPPET;
    }
    dynamicVxml += VXML_END ;

        // Write out the dynamic VXML script to be processed by OpenVXI
        if (out)
        {
        out->remove(0);
        UtlString responseHeaders;
        MailboxManager::getResponseHeaders(dynamicVxml.length(), responseHeaders);

        out->append(responseHeaders.data());
        out->append(dynamicVxml.data());
        }
        return OS_SUCCESS;
}


OsStatus
GetAllSystemPromptsCGI::getSystemGreetings(UtlString& vxmlSnippet)
{
    OsStatus result = OS_FAILED ;
    UtlString greetingUrl;
    vxmlSnippet = "";

        MailboxManager* pMailboxManager = MailboxManager::getInstance();

        // Get the URL of the standard greeting.
        if( pMailboxManager->getSystemPromptUrl(STANDARD_SYSTEM_GREETING, greetingUrl, FALSE ) == OS_SUCCESS )
                vxmlSnippet += "<var name=\"standard\" expr=\"'" + greetingUrl + "'\" />\n" ;
        else
                vxmlSnippet += "<var name=\"standard\" expr=\"'-1'\" />\n" ;


        // Get the URL of the after hours greeting.
        if( pMailboxManager->getSystemPromptUrl(AFTER_HOURS_SYSTEM_GREETING, greetingUrl, FALSE ) == OS_SUCCESS )
                vxmlSnippet += "<var name=\"afterhours\" expr=\"'" + greetingUrl + "'\" />\n" ;
        else
                vxmlSnippet += "<var name=\"afterhours\" expr=\"'-1'\" />\n" ;


    // Get the URL of the special occasion greeting
        if( pMailboxManager->getSystemPromptUrl(SPECIAL_OCCASION_SYSTEM_GREETING, greetingUrl, FALSE ) == OS_SUCCESS )
                vxmlSnippet += "<var name=\"special\" expr=\"'" + greetingUrl + "'\" />\n" ;
        else
                vxmlSnippet += "<var name=\"special\" expr=\"'-1'\" />\n" ;


    // Get the URL of the generic greeting.
        if( pMailboxManager->getSystemPromptUrl(GENERIC_SYSTEM_GREETING, greetingUrl, FALSE ) == OS_SUCCESS )
                vxmlSnippet += "<var name=\"generic\" expr=\"'" + greetingUrl + "'\" />\n" ;
        else
                vxmlSnippet += "<var name=\"generic\" expr=\"'-1'\" />\n" ;


    if( vxmlSnippet != "" )
    {
        vxmlSnippet +=  "<return namelist=\"standard afterhours special generic\" />\n" ;
        result = OS_SUCCESS ;
    }

    return result ;

}


OsStatus
GetAllSystemPromptsCGI::getAutoAttendantPrompts(UtlString& vxmlSnippet)
{
    OsStatus result = OS_FAILED ;
    UtlString greetingUrl;
    vxmlSnippet = "";

        MailboxManager* pMailboxManager = MailboxManager::getInstance();

        // Get the URL of the custom recorded auto attendant prompt.
        if( pMailboxManager->getSystemPromptUrl(RECORDED_AUTOATTENDANT_PROMPT, greetingUrl, FALSE ) == OS_SUCCESS )
                vxmlSnippet += "<var name=\"custom\" expr=\"'" + greetingUrl + "'\" />\n" ;
        else
                vxmlSnippet += "<var name=\"custom\" expr=\"'-1'\" />\n" ;


        // Get the URL of the generic auto attendant prompt.
        if( pMailboxManager->getSystemPromptUrl(GENERIC_AUTOATTENDANT_PROMPT, greetingUrl, FALSE ) == OS_SUCCESS )
                vxmlSnippet += "<var name=\"generic\" expr=\"'" + greetingUrl + "'\" />\n" ;
        else
                vxmlSnippet += "<var name=\"generic\" expr=\"'-1'\" />\n" ;


    if( vxmlSnippet != "")
    {
        vxmlSnippet +=  "<return namelist=\"custom generic\" />\n" ;
        result = OS_SUCCESS ;
    }

    return result ;

}




OsStatus
GetAllSystemPromptsCGI::handleWebRequest(UtlString* out)
{
    // Nothing to do here for this release.
    return OS_SUCCESS ;
}
