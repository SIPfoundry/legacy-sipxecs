//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef HTMLDEFS_H
#define HTMLDEFS_H

// SYSTEM INCLUDES
//#include <...>
#include "os/OsStatus.h"
#include "os/OsDefs.h"



// APPLICATION INCLUDES
#include "mailboxmgr/VXMLCGICommand.h"

#define NO_HTML_CACHE_HEADER "Content-type: text/html\n" \
    "pragma: no-cache\n\n"

#define HTML_BEGIN NO_HTML_CACHE_HEADER \
    "<HTML>\n" \
    "<HEAD>\n" \
        "<TITLE>MediaServer User UI</TITLE>\n" \
        "<link rel=\"stylesheet\" href=\"/style/voicemail.css\" type=\"text/css\">\n" \
        "<SCRIPT src=\"/userui/jsFunctions.js\"></SCRIPT>\n" \
    "</HEAD>\n" \
    "<BODY class=\"bglight\">\n"

#define HTML_BEGIN_WITH_ONLOAD  NO_HTML_CACHE_HEADER \
    "<HTML>\n" \
    "<HEAD>\n" \
        "<TITLE>MediaServer User UI</TITLE>\n" \
        "<link rel=\"stylesheet\" href=\"/style/voicemail.css\" type=\"text/css\">\n" \
        "<SCRIPT src=\"/userui/jsFunctions.js\"></SCRIPT>\n" \
    "</HEAD>\n" \
    "<BODY class=\"bglight\" "


#define HTML_END "</BODY>\n" \
                                 "</HTML>"

#define CGI_URL                         "/cgi-bin/voicemail/mediaserver.cgi"

// URLs for different destinations
#define VOICEMAIL_NOT_ENABLED_URL       "/userui/voicemail_not_enabled.html"
#define VOICEMAIL_NOT_ENABLED_NAV_URL   "/userui/voicemail_not_enabled_nav.html"
#define NAV_URL                         "/userui/navigation.html"

#define REDIRECT_SCRIPT         "setTimeout('location.href=\""

#define REDIRECT_SCRIPT_BEGIN   "<SCRIPT language=\"JavaScript\">\n" \
                                                    "<!-- \n" \
                                                            REDIRECT_SCRIPT

#define REDIRECT_SCRIPT_END     "\"', 0);\n" \
                                                    "// -->\n" \
                                                    "</SCRIPT>\n"

#define WEBPAGE_SNIPPET1        "<table width=\"85%\" border=\"0\">\n" \
                                    "<tr>\n" \
                                        "<td class=\"formtitle\" height=\"30\" width=\"92%\">"

#define WEBPAGE_SNIPPET2        "</td>\n" \
                                "<td align=\"right\" width=\"8%\">&nbsp;<a class=\"bgdark\" href=\"javascript:void 0\" onclick=\"displayHelpPage('/userui/WebHelp/mediauser.htm#"

#define WEBPAGE_SNIPPET3        "');\">Help</a></td>\n" \
                                "</tr>\n" \
                                "<tr>\n" \
                                  "<td class=\"errortext_light\" colspan=\"2\">\n" \
                                    "<hr class=\"dms\">\n" \
                                  "</td>\n" \
                                "</tr>\n" \
                                "<tr>\n" \
                                  "<td class=\"errortext_light\" colspan=\"2\">\n"

#define WEBPAGE_SNIPPET4        "</td>\n" \
                                "</tr>\n" \
                                "<tr> \n" \
                                  "<td colspan=\"2\"> \n" \
                                    "<table border=\"0\" align=\"left\" width=\"100%\">\n" \
                                      "<tr>\n" \
                                        "<td colspan=\"2\"> \n" \
                                          "<table class=\"bglist\" cellspacing=\"1\" cellpadding=\"4\" border=\"0\" width=\"100%\">\n"

#define DELETE_CONFIRMATION     "Do you want to proceed with the delete?"

#define DELETE_MSG_CONFIRMATION "Once deleted, messages cannot be restored. Do you want to proceed?"

#define PROTOCOL_MISMATCH       "<div class=\"errortext_light\">Mediaserver URL is not setup correctly (http vs https)</div>\n"

#define RECORD_GREETINGS_NOTE   "<tr><td class=\"notetext\"><b>Note:</b>To record greetings, log in to voicemail on your phone. Select voicemail options to record your name or greetings.</td></tr>"

#define REFRESH_SCRIPT          "<SCRIPT language=\"JavaScript\">\n" \
                                                    "<!-- \n" \
                                                            "reloadMainFrame() ;"

#define EMBED_MEDIAPLAYER_PLUGIN    "<SCRIPT language=\"JavaScript\">\n" \
                                                        "<!-- \n" \
                                                                "embedMediaPlayer(); \n" \
                                    "// -->\n" \
                                                        "</SCRIPT>\n"


// Status codes returned by different CGIs.
#define MOVE_MSG_FAILED                 "1"
#define RECYCLE_DELETED_MESSAGES_FAILED "2"
#define EDIT_MSG_SUCCESSFUL             "3"
#define MOVE_MSG_NOT_SELECTED           "4"
#define MOVE_FOLDER_NOT_SELECTED        "5"
#define SET_ACTIVE_GREETING_SUCCESS     "6"
#define SET_ACTIVE_GREETING_FAILED      "7"
#define DELETE_GREETING_SUCCESS         "8"
#define DELETE_GREETING_FAILED          "9"
#define ADD_FOLDER_SUCCESS              "10"
#define ADD_FOLDER_FAILED               "11"
#define EDIT_FOLDER_SUCCESS             "12"
#define EDIT_FOLDER_FAILED              "13"
#define DELETE_FOLDER_SUCCESS           "14"
#define DELETE_FOLDER_FAILED            "15"
#define ADD_NOTIFICATION_FAILED         "16"
#define EDIT_NOTIFICATION_FAILED        "17"
#define DELETE_NOTIFICATION_FAILED      "18"
#define ADD_NOTIFICATION_SUCCESS        "19"
#define EDIT_NOTIFICATION_SUCCESS       "20"
#define DELETE_NOTIFICATION_SUCCESS     "21"
#define NOTIFICATION_DUPLICATE_FOUND    "22"
#define INVALID_NOTIFICATION_ADDRESS    "23"
#define MSG_MOVED_SUCCESSFULLY          "25"
#define MSG_DELETED_SUCCESSFULLY        "27"
#define DELETE_MSG_FAILED               "28"
#define MOVE_MSG_AND_FOLDER_NOT_SELECTED "29"
#define DELETE_MSG_NOT_SELECTED         "30"
#define RECYCLE_DELETED_MSG_SUCCESSFUL  "31"
#define RECYCLE_DELETED_MSG_NOT_SELECTED "32"
#define ADD_DISTRIBUTION_FAILED         "33"
#define EDIT_DISTRIBUTION_FAILED        "34"
#define DELETE_DISTRIBUTION_FAILED      "35"
#define ADD_DISTRIBUTION_SUCCESS        "36"
#define EDIT_DISTRIBUTION_SUCCESS       "37"
#define DELETE_DISTRIBUTION_SUCCESS     "38"
#define DISTRIBUTION_DUPLICATE_FOUND    "39"
#define INVALID_DISTRIBUTION_ADDRESS    "40"




// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#endif //HTMLDEFS_H
