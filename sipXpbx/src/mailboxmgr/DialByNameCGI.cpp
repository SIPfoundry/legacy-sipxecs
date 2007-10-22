//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "net/Url.h"
#include "sipdb/ResultSet.h"
#include "sipdb/DialByNameDB.h"
#include "sipdb/ExtensionDB.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ActiveGreetingHelper.h"
#include "mailboxmgr/DialByNameCGI.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

DialByNameCGI::DialByNameCGI( const Url& from,
                              const UtlString& digits,
                              const UtlBoolean& invokedByDeposit):
    m_from ( from ),
    m_digits ( digits ),
    m_invokedByDepositFlag( invokedByDeposit )
{}

DialByNameCGI::~DialByNameCGI()
{}

OsStatus
DialByNameCGI::execute( UtlString* out )
{
    OsStatus result = OS_SUCCESS;
    UtlString vxmlOutput = getVXMLHeader();
    UtlString urlSeparator ( URL_SEPARATOR );

    UtlString ivrPromptUrl;

    MailboxManager::getInstance()->getIvrPromptURL( ivrPromptUrl );

    // Make sure that the user entered some digits
    if ( m_digits.length() > 0 )
    {
        // Find the matches in the IMDB for the dialed digits.
        ResultSet dialByNameEntries;
        DialByNameDB::getInstance()->
            getContacts( m_digits, dialByNameEntries );

            // Get the mailbox id for each match.
        int numMatches = dialByNameEntries.getSize();

        vxmlOutput += VXML_TIMEOUT_PROPERTY_START +
                      UtlString("7s") +
                      VXML_TIMEOUT_PROPERTY_END ;

        // Construct an ActiveGreetingHelper object to get the greeting contents
        // from each of the users that was looked up in the database
        ActiveGreetingHelper activeGreetingHelper;

        if ( numMatches > 0 )
        {
            char temp [32];
            sprintf ( temp, "%d", numMatches );
            UtlString menuPrompts, menuChoices, formFields, nextNum;

            UtlString menu = "<menu id=\"results\" dtmf=\"true\">";
                int promptIndex = 1;
            UtlString greetingUrl, greetingVxml;

            // 'maxChoicesAllowed' defines the maximum number of matches that will
            // be played to the user. Currently, we provide only the first
            // DIALBYNAME_MAX_BLOCKSIZE matches.
            // int maxChoicesAllowed = numMatches ;
            int maxChoicesAllowed = DIALBYNAME_MAX_BLOCKSIZE;
            if( numMatches < DIALBYNAME_MAX_BLOCKSIZE )
            {
                maxChoicesAllowed = numMatches ;
            }

            for ( int i = 0; i< maxChoicesAllowed; i++ )
            {
                UtlHashMap record;
                dialByNameEntries.getIndex( i, record );
                UtlString np_identityKey ("np_identity");
                UtlString np_digitsKey ("np_digits");
                UtlString identity = *((UtlString*)record.findValue(&np_identityKey));
                UtlString np_digits = *((UtlString*)record.findValue(&np_digitsKey));

                Url identityUrl( identity );
                UtlString userid, domain;
                identityUrl.getUserId( userid );
                identityUrl.getHostAddress( domain );

                if ( !userid.isNull() && !domain.isNull() )
                {
                    // get the dynamically generated greeting name from for the
                    // next mailbox in the list, only update the menus if we can
                    // fetch a greeting for that identity
                    if ( activeGreetingHelper.getRecordedName ( identity, greetingUrl, FALSE ) == OS_SUCCESS )
                                        {
                                                greetingVxml = "<prompt><audio src=\"" + greetingUrl + "\"/></prompt>";
                    } else
                                        {
                        // find the user's optional extension or username and read it out
                        UtlString useridOrExtension;
                        if ( !ExtensionDB::getInstance()->
                                getExtension( identityUrl, useridOrExtension ) )
                        {
                            // could not resolve extension, use the userid instead
                            useridOrExtension = userid;
                        }

                        // User does not have a numeric extension
                        // so parse for the userid portion of their identity
                        if( useridOrExtension.first( '@' ) != UTL_NOT_FOUND )
                        {
                            useridOrExtension = useridOrExtension(0, useridOrExtension.first( '@' ));
                        }

                        useridOrExtension.toLower();
                        greetingVxml =
                            "<prompt><audio src=\"" +
                                ivrPromptUrl +
                                urlSeparator +
                                UtlString (PROMPT_ALIAS) +
                                urlSeparator + "extension.wav\"/></prompt>" \
                            "<prompt><say-as type=\"acronym\">" + useridOrExtension + "</say-as></prompt>" ;
                                        }

                    sprintf ( temp, "%d", promptIndex );
                    nextNum = temp;

                    // Create the "press {N} for {Greeting}" series of prompts
                    menuPrompts +=
                        "<prompt><audio src=\"" +
                            ivrPromptUrl +
                            urlSeparator +
                            UtlString (PROMPT_ALIAS) +
                            urlSeparator + "press.wav\"/></prompt>"
                        "<prompt><say-as type=\"number\">" + nextNum + "</say-as></prompt>"
                        "<prompt><audio src=\"" +
                            ivrPromptUrl +
                            urlSeparator +
                            UtlString (PROMPT_ALIAS) +
                            urlSeparator + "for.wav\"/></prompt>" +
                        greetingVxml;

                                        menuChoices +=
                        "<choice dtmf=\"" + nextNum +
                        "\" next=\"#formname" + nextNum + "\"/>";

                    if( m_invokedByDepositFlag )
                    {
                        formFields +=   "<form id=\"formname" + nextNum + "\">" \
                                        "<block>\n" \
                                        "<var name=\"extension\" expr=\"'" + identity + "'\"/>\n" \
                                        "<return namelist=\"extension\"/>\n" \
                                        "</block>\n" \
                                        "</form>\n" ;
                    } else
                    {
                                            formFields +=
                            "<form id=\"formname" + nextNum + "\">" \
                                "<block> \n" \
                                    "<prompt bargein=\"false\"> \n" \
                                        "<audio src=\"" + ivrPromptUrl + "/" + PROMPT_ALIAS + "/please_hold.wav\"/> \n" \
                                    "</prompt> \n" \
                                "</block> \n" \
                                "<transfer dest=\"sip:" + identity + "\"/> \n" \
                            "</form>";
                    }
                    promptIndex++;
                }
            }


            if( promptIndex == 1 )
            {
                // Valid matches were not found
                // Create a script indicating that there was no match
                vxmlOutput +=
                    "<form> \n" \
                    "<block> \n" \
                        "<prompt bargein=\"false\"> \n" \
                            "<audio src=\"" +
                                ivrPromptUrl + urlSeparator + UtlString (PROMPT_ALIAS) + urlSeparator +
                                "dialbyname_no_match.wav\"/> \n" \
                        "</prompt>" ;

                if( m_invokedByDepositFlag )
                {
                    vxmlOutput +=   "<var name=\"result\" expr=\"'failed'\"/> "\
                                    "<var name=\"extension\" expr=\"'-1'\"/>\n" \
                                    "<return namelist=\"extension result\"/>\n" ;
                } else
                {
                    vxmlOutput += VXML_FAILURE_BLOCK;
                }
                vxmlOutput += "</block> \n</form>" ;
            } else
            {
                menuChoices +=
                    "<choice dtmf=\"*\" next=\"#tryagain\"/>" \
                    "<choice dtmf=\"#\" next=\"#repromptmenu\"/>" ;

                menuPrompts +=
                     "<prompt><audio src=\"" +
                         ivrPromptUrl +
                         urlSeparator +
                         UtlString (PROMPT_ALIAS) +
                         urlSeparator + "enter_different_name.wav\"/></prompt>";

                menu += menuPrompts + menuChoices;
                menu += "<nomatch>" \
                        "<prompt bargein=\"false\"> \n" \
                            "<audio src=\"" +
                                ivrPromptUrl + urlSeparator + UtlString (PROMPT_ALIAS) + urlSeparator +
                                "no_entry_matches.wav\"/> \n" \
                        "</prompt>" \
                        "<reprompt/>"\
                        "</nomatch> \n";
                menu += "<noinput count=\"3\"> \n" \
                            "<prompt bargein=\"false\"> \n" \
                                "<audio src=\"" +
                                    ivrPromptUrl + urlSeparator + UtlString (PROMPT_ALIAS) + urlSeparator +
                                    "thankyou_goodbye.wav\" /> \n" \
                            "</prompt> \n" \
                            "<disconnect /> \n" \
                        "</noinput> \n" \
                        "</menu>" ;

                vxmlOutput += menu;
                formFields += "<form id=\"tryagain\"> <block> \n" \
                                    "<prompt bargein=\"false\"> \n" \
                                        "<audio src=\"" +
                                            ivrPromptUrl + urlSeparator + UtlString (PROMPT_ALIAS) + urlSeparator +
                                            "cancelled.wav\"/> \n" \
                                        "</prompt>" \
                                    "<var name=\"result\" expr=\"'failed'\"/>\n" \
                                    "<var name=\"extension\" expr=\"'-1'\"/>\n" \
                                        "<return namelist=\"extension result\"/>\n"
                              "</block></form>\n";

                formFields += "<form id=\"repromptmenu\"> \n" \
                                "<block>\n" \
                                "<prompt bargein=\"false\"> \n" \
                                "<audio src=\"" +
                                    ivrPromptUrl + urlSeparator + UtlString (PROMPT_ALIAS) + urlSeparator +
                                    "no_entry_matches.wav\"/> \n" \
                                "</prompt>" \
                                "<goto next=\"#results\"/> \n"\
                                "</block> \n" \
                                "</form>\n";

                               vxmlOutput += formFields;
            }
        } else
        {
            // Create a script indicating that there was no match
            vxmlOutput +=
                    "<form> \n" \
                    "<block> \n" \
                        "<prompt bargein=\"false\"> \n" \
                            "<audio src=\"" +
                                ivrPromptUrl + urlSeparator + UtlString (PROMPT_ALIAS) + urlSeparator +
                                "dialbyname_no_match.wav\"/> \n" \
                        "</prompt>" ;

            if( m_invokedByDepositFlag )
            {
                vxmlOutput +=   "<var name=\"result\" expr=\"'success'\"/> "\
                                "<var name=\"extension\" expr=\"'-1'\"/>\n" \
                                "<return namelist=\"extension result\"/>\n" ;
            }
            else
               vxmlOutput += VXML_FAILURE_BLOCK;

            vxmlOutput += "</block> \n</form>" ;

        }
    } else
    {
        // Create a script indicating that there was no match
        vxmlOutput +=
            "<form><block><prompt bargein=\"false\"><audio src=\"" +
            ivrPromptUrl + urlSeparator + UtlString (PROMPT_ALIAS) + urlSeparator +
            "dialbyname_no_match.wav\"/></prompt>";
        vxmlOutput +=
            "<prompt bargein=\"false\"><audio src=\"" +
            ivrPromptUrl + urlSeparator + UtlString (PROMPT_ALIAS) + urlSeparator +
            "dialbyname.wav\"/></prompt>"\
            VXML_FAILURE_BLOCK \
            "</block></form>";
    }
    // Terminate the VMXL
    vxmlOutput += VXML_END;

    if ( out )
        {
        out->remove(0);
        UtlString responseHeaders;
        MailboxManager::getResponseHeaders(vxmlOutput.length(), responseHeaders);

        out->append(responseHeaders.data());
        out->append(vxmlOutput.data());
        }
    return result;
}
