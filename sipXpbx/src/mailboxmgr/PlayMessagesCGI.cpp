//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <ctype.h>

// APPLICATION INCLUDES
#include "net/Url.h"
#include "net/HttpMessage.h"
#include "os/OsSysLog.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/PlayMessagesCGI.h"
#include "mailboxmgr/GetMessageDetailsHelper.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/ValidateMailboxCGIHelper.h"
#include "mailboxmgr/ActiveGreetingHelper.h"
#include "os/OsDateTime.h"
#include "utl/UtlSortedList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

PlayMessagesCGI::PlayMessagesCGI(
   const UtlBoolean& requestIsFromWebUI,
   const UtlString& mailbox,
   const Url& from,
   const char* category,
   const char* blockSize,
   const char* nextblockhandle,
   const char* status,
   const char* unheardMsgIdList,
   const char* frameset) :
   m_mailboxIdentity ( mailbox ),
   m_from ( from ),
   m_category( category),
   m_iBlockSize ( atoi(blockSize) ),
   m_iNextBlockHandle ( atoi(nextblockhandle) ),
   m_strNextBlockHandle (nextblockhandle),
   m_fromWeb( requestIsFromWebUI ),
   m_status( status ),
   m_unheardMsgIdList( unheardMsgIdList ),
   m_frameSet( frameset )
{
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "PlayMessagesCGI::PlayMessagesCGI(m_fromWeb = %d, m_mailboxIdentity = '%s', m_from = '%s', m_category = '%s', m_iBlockSize = %d, m_iNextBlockHandle = %d, m_status = '%s', m_unheardMsgIdList = '%s', m_frameSet = '%s') called",
                 m_fromWeb, m_mailboxIdentity.data(), m_from.toString().data(),
                 m_category.data(), m_iBlockSize, m_iNextBlockHandle,
                 m_status.data(), m_unheardMsgIdList.data(),
                 m_frameSet.data());
}

PlayMessagesCGI::~PlayMessagesCGI()
{}

OsStatus
PlayMessagesCGI::execute(UtlString* out)
{
   // Validate the mailbox id and extension.
   ValidateMailboxCGIHelper validateMailboxHelper ( m_mailboxIdentity );
   OsStatus result = validateMailboxHelper.execute( out );
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "PlayMessagesCGI::execute: validateMailboxHelper.execute() returns %d",
                 result);
   if( result == OS_SUCCESS )
   {
      validateMailboxHelper.getMailboxIdentity( m_mailboxIdentity );
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "PlayMessagesCGI::execute: getMailboxIdentity returns m_mailboxIdentity = '%s'",
                    m_mailboxIdentity.data());
      if( m_fromWeb )
         result = handleWebRequest( out );
      else
         result = handleOpenVXIRequest( out );
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "PlayMessagesCGI::execute: m_fromWeb = %d, handle{Web,OpenVXI}Request returns %d, out = '%s'",
                    m_fromWeb, result, out->data());
   }

   return result;
}

OsStatus
PlayMessagesCGI::handleWebRequest( UtlString* out )
{
   // If this request was from user clicking on the "Show Voicemail Inbox"
   // in the email notification.
   if (m_frameSet == "yes")
   {
      return handlePlayFromEmailNotification(out);
   }

   UtlString cgiURL, dynamicHTML;
   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   // Get the mediaserver base URL
   OsStatus result = pMailboxManager->getMediaserverURLForWeb( cgiURL );
   if( result == OS_SUCCESS )
   {
      // URL of the mediaserver CGI
      cgiURL += CGI_URL;

      // Get the page title.
      UtlString title;
      getPageTitle( title );

      // Ignore the 'blocksize' received from the request.
      // Override it with the value set in the voicemail.xml file.
      pMailboxManager->getMessageBlockSizeForWeb( m_iBlockSize );

      // Get error message
      UtlString statusStr = "&nbsp;";
      if( m_status == MSG_MOVED_SUCCESSFULLY )
         statusStr = "Message(s) moved successfully.";
      else if( m_status == MSG_DELETED_SUCCESSFULLY )
         statusStr = "Message(s) deleted successfully.";
      else if( m_status == MOVE_MSG_FAILED )
         statusStr = "Failed to move the message(s).";
      else if( m_status == DELETE_MSG_FAILED )
         statusStr = "Failed to delete the message(s).";
      else if( m_status == RECYCLE_DELETED_MESSAGES_FAILED )
         statusStr = "Failed to empty the deleted folder.";
      else if( m_status == RECYCLE_DELETED_MSG_SUCCESSFUL )
         statusStr = "Message(s) deleted successfully.";
      else if( m_status == RECYCLE_DELETED_MSG_NOT_SELECTED )
         statusStr = "Please select the message(s) to be deleted";
      else if( m_status == EDIT_MSG_SUCCESSFUL )
         statusStr = "Message edited successfully";
      else if( m_status == MOVE_MSG_NOT_SELECTED )
         statusStr = "Please select the message(s) to be moved";
      else if( m_status == MOVE_FOLDER_NOT_SELECTED )
         statusStr = "Please select the destination folder";
      else if( m_status == MOVE_MSG_AND_FOLDER_NOT_SELECTED )
         statusStr = "Please select the message(s) to be moved and the destination folder";
      else if( m_status == DELETE_MSG_NOT_SELECTED )
         statusStr = "Please select the message(s) to be deleted";


      // Get the time in minutes after which the page should refresh automatically.
      UtlString refreshInterval;
      pMailboxManager->getPageRefreshInterval( refreshInterval );


      // Construct the output HTML string.
      dynamicHTML =   NO_HTML_CACHE_HEADER \
         "<HTML>\n" \
         "<HEAD>\n" \
         "<TITLE>MediaServer User UI</TITLE>\n" \
         "<link rel=\"stylesheet\" href=\"/style/voicemail.css\" type=\"text/css\">\n" \
         "<SCRIPT src=\"/userui/jsFunctions.js\"></SCRIPT>\n" \
         "<script language=\"JavaScript\"> \n" \
         "var countDownInterval=" + refreshInterval + "*60; \n" \
         "var countDownTime=countDownInterval+1; \n" \
         "function countDown(){ \n" \
         "countDownTime--; \n" \
         "if (countDownTime <=0){ \n" \
         "countDownTime=countDownInterval; \n" \
         "clearTimeout(counter); \n" \
         "window.location.reload(); \n" \
         "return; \n" \
         "} \n" \
         "counter=setTimeout(\"countDown()\", 1000); \n" \
         "} \n" \
         "if (document.all||document.getElementById) \n" \
         "countDown(); \n" \
         "else \n" \
         "window.onload=countDown; \n" \
         "</script> \n" \
         "</HEAD>\n" \
         "<BODY class=\"bglight\">\n" \
         EMBED_MEDIAPLAYER_PLUGIN \
         "<form name=messageList action=\"" + cgiURL + "\">\n " \
         "<table width=\"600\" border=\"0\" class=\"bglight\">\n" \
         "<tr>\n" \
         "<td class=\"formtitle\" height=\"30\" width=\"92%\">" + title + "</td>\n" \
         "<td align=\"right\" width=\"8%\">&nbsp;<a class=\"greentext\" href=\"javascript:void 0\" onclick=\"displayHelpPage('/userui/WebHelp/mediauser.htm#all_folders_managing_messages.htm');\">Help</a></td>\n" \
         "</tr>\n" \
         "<tr>\n" \
         "<td class=\"errortext_light\" colspan=\"2\">\n" \
         "<hr class=\"dms\">\n" \
         "</td>\n" \
         "</tr>\n" \
         "<tr> \n" \
         "<td class=\"errortext_light\" colspan=\"2\"> \n" + statusStr + "</td>\n" \
         "</tr>\n";

      UtlString generatedContent;
      result = getHtmlContent( generatedContent, cgiURL );

      if( result == OS_SUCCESS )
      {
         dynamicHTML += generatedContent;

      } else if( m_iNextBlockHandle != -1 )
      {
         m_iNextBlockHandle = -1;
         m_strNextBlockHandle = "-1";
         generatedContent = "";
         result = getHtmlContent( generatedContent, cgiURL );
         if( result == OS_SUCCESS )
            dynamicHTML += generatedContent;
      }

      if( result != OS_SUCCESS )
      {
         // Messages are not available for the given category and lastMessageHandle.
         dynamicHTML +=
            "<tr>\n" \
            "<td class=\"errortext_light\" colspan=\"2\">\n" \
            "You have no messages in this folder\n" \
            "</td>\n" \
            "</tr>\n";

      }

      dynamicHTML +=
         "</table>\n" \
         "</td>\n" \
         "</tr>\n" \
         "</table> \n" \
         "</form>";
   } else
   {
      dynamicHTML =   HTML_BEGIN \
         PROTOCOL_MISMATCH;
   }


   dynamicHTML += HTML_END;

   // Write out the dynamic VXML script to be processed by OpenVXI
   if (out)
   {
      out->remove(0);
      out->append(dynamicHTML.data());
   }
   return OS_SUCCESS;

}

OsStatus
PlayMessagesCGI::handleOpenVXIRequest( UtlString* out )
{
   // Construct the helper object that will retrieve the message details.
   GetMessageDetailsHelper gmdHelper;

   // Mediaserver base URL - http://mediaserver:6000
   UtlString ivrPromptUrl = gmdHelper.getIvrPromptUrl();
   UtlString mediaserverUrl = gmdHelper.getMediaserverUrl();
   UtlString secureMediaserverUrl = gmdHelper.getMediaserverSecureUrl();

   UtlString promptUrl = ivrPromptUrl + URL_SEPARATOR + PROMPT_ALIAS + URL_SEPARATOR;
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "PlayMessagesCGI::handleOpenVXIRequest: mediaserverUrl = '%s', secureMediaserverUrl = '%s', promptUrl = '%s'",
                 mediaserverUrl.data(),
                 secureMediaserverUrl.data(),
                 promptUrl.data());

   // Message details to be retrieved
   UtlString url = "url";
   UtlString timestamp = "timestamp";
   UtlString id = "id";
   UtlString msgid = "messageid";
   UtlString from = "from";

   // Flag to indicate if all messages have been retrieved.
   UtlBoolean endOfMessages;

   UtlString dynamicVxml (VXML_BEGIN_WITH_ROOT);
   dynamicVxml +=  mediaserverUrl + URL_SEPARATOR +
      VOICEMAIL_SCRIPTS_ALIAS + URL_SEPARATOR +
      "root.vxml\"> \n";

   // Retrieve the message block.
   if ( gmdHelper.getMessageBlock(
           m_mailboxIdentity,
           m_category,
           m_iBlockSize,
           m_iNextBlockHandle,
           endOfMessages,
           FALSE,
           FALSE) == OS_SUCCESS )
   {
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "PlayMessagesCGI::handleOpenVXIRequest: getMessageBlock succeeds, m_mailboxIdentity = '%s', m_category = '%s', m_iBlockSize = %d, m_iNextBlockHandle = %d, endOfMessages = %d",
                    m_mailboxIdentity.data(), m_category.data(), m_iBlockSize,
                    m_iNextBlockHandle, endOfMessages);

      // iterator for looping through all the messages in the block.
      int iterator = 1;
      dynamicVxml += VXML_TIMEOUT_PROPERTY_START;
      dynamicVxml += "10s";
      dynamicVxml +=  VXML_TIMEOUT_PROPERTY_END \
         "<form> \n" \
         "<var name=\"messageids\" /> \n" \
         "<var name=\"messageidlist\" expr=\"'0'\"/> \n" \
         "<var name=\"action\" /> \n" \
         "<var name=\"fromfolder\" /> \n" \
         "<var name=\"tofolder\" /> \n" \
         "<var name=\"nextfield\" /> \n" \
         "<var name=\"transferto\" /> \n" \
         "<var name=\"maintainstatus\" expr=\"'no'\" /> \n" \
         "<!-- Flag to indicate if the message should be played. \n" \
         "     For example, if there is no user input, should replay only the options. -->\n" \
         "<var name=\"playmsg\" expr=\"'yes'\" /> \n" \
         "<var name=\"mailbox\" expr=\"'" + m_mailboxIdentity + "'\" /> \n" \
         "<var name=\"updatestatuscalledfrom\" /> \n" \
         "<var name=\"replytovmmailbox\" /> \n";

      UtlString nextfield, messageIdList, messageId, nextblock;

      // Get information about each message retrieved.
      while (gmdHelper.getNextMessageCollectable() == OS_SUCCESS )
      {
         OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                       "PlayMessagesCGI::handleOpenVXIRequest: in loop, iterator = %d",
                       iterator);

         // String for storing the iterator
         char iteratorStr[5];

         // Convert the iterator into string.
         // Result will be stored in iteratorStr
         sprintf ( iteratorStr, "%d", iterator );

         UtlString fieldname = "msg";
         fieldname += iteratorStr;

         UtlString blockname = "blck";
         blockname += iteratorStr;

         // Get next field name.
         sprintf ( iteratorStr, "%d", iterator + 1);
         nextfield = "msg";
         nextfield += iteratorStr;

         nextblock = "blck";
         nextblock += iteratorStr;

         // Variable to indicate if the caller was an internal SIPxchange user.
         OsStatus callerIsSipxUser = OS_FAILED;

         // Boolean to indicate the voicemail status of the caller
         // who left the voicemail
         UtlBoolean voicemailEnabled = FALSE;

         // Variables to hold information about the caller who left voicemail
         UtlString fromExtension, fromMailboxIdentity;

         UtlString prompts, promptMsgEnvelope;

         OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                       "PlayMessagesCGI::handleOpenVXIRequest: fieldname = '%s', blockname = '%s', nextfield = '%s', nextblock = '%s', callerIsSipxUser = %d, voicemailEnabled = %d",
                       fieldname.data(), blockname.data(), nextfield.data(),
                       nextblock.data(), callerIsSipxUser, voicemailEnabled);

         while( gmdHelper.getNextMessage() == OS_SUCCESS )
         {
            messageId = gmdHelper.getMessageDetails(msgid);
            if( m_category == "heard" )
               messageIdList = m_unheardMsgIdList;
            else
               messageIdList = messageId + ";";
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "PlayMessagesCGI::handleOpenVXIRequest: messageId = '%s', messageIdList = '%s'",
                          messageId.data(), messageIdList.data());

            if( prompts == "" )
            {
               dynamicVxml     +=  "<block name=\"" + blockname +"\" > \n "\
                  "<assign name=\"messageids\" expr=\"'" + messageId + "'\" />" \
                  "<assign name=\"messageidlist\" expr=\"'" + messageIdList + "'\" />" \
                  "</block>\n";

               // extract url "from" field from the XML file.
               UtlString vxmlFriendlyFrom =
                  gmdHelper.getMessageDetails( from );

               UtlString unformattedFrom ( vxmlFriendlyFrom );

               // check if the caller was a sipxchange user with voicemail enabled.
               // use this to enable "reply to voicemail" option and
               // to construct appropriate envelope information

               callerIsSipxUser = validateFromUrl(unformattedFrom,
                                                  voicemailEnabled,
                                                  fromExtension,
                                                  fromMailboxIdentity );
               OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                             "PlayMessagesCGI::handleOpenVXIRequest: validateFromUrl(unformattedFrom = '%s') returns callerIsSipxUser = %d, voicemailEnabled = %d, fromExtension = '%s', fromMailboxIdentity = '%s'",
                             unformattedFrom.data(), callerIsSipxUser,
                             voicemailEnabled, fromExtension.data(),
                             fromMailboxIdentity.data());

               // Generate the message envelope information
               promptMsgEnvelope = getTimestampVxml (gmdHelper.getMessageDetails(timestamp), promptUrl ) +
                  getMsgOriginVxml (    callerIsSipxUser,
                                        voicemailEnabled,
                                        fromExtension,
                                        fromMailboxIdentity,
                                        unformattedFrom,
                                        promptUrl);
               OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                             "PlayMessagesCGI::handleOpenVXIRequest: promptMsgEnvelope = '%s'",
                             promptMsgEnvelope.data());

               // check if message envelope should be played before the message
               if( gmdHelper.enableVoicemailInfoPlayback() )
                  prompts = promptMsgEnvelope;

               prompts +=       "<prompt cond=\"playmsg=='yes'\">\n" \
                  "<audio src=\"" + gmdHelper.getMessageDetails("playlisturl") + "\" /> \n";
            }
         }
         prompts += "</prompt>\n";

         UtlString messageMenuPrompt = getMessageMenuVxml(promptUrl,
                                                          callerIsSipxUser,
                                                          voicemailEnabled);
         OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                       "PlayMessagesCGI::handleOpenVXIRequest: getMessageMenuVxml(promptUrl = '%s', callerIsSipxUser = %d, voicemailEnabled = %d) returns messageMenuPrompt = '%s'",
                       promptUrl.data(), callerIsSipxUser, voicemailEnabled,
                       messageMenuPrompt.data());

         // Construct the VXML for playing this message
         dynamicVxml +=
            "<field name=\"" + fieldname + "\" type=\"digits?maxlength=1;minlength=1\"> \n" +
            prompts +
            messageMenuPrompt +
            "<filled> \n" \
            "<assign name=\"playmsg\" expr=\"'yes'\" />\n" \
            "<if cond=\"" + fieldname + "== '1'\"> \n" +
            promptMsgEnvelope +
            "<assign name=\"playmsg\" expr=\"'no'\" />\n" \
            "<clear namelist=\"" + fieldname + "\" />\n" \
            "<goto nextitem=\"" + fieldname + "\" /> \n" \
            "<elseif cond=\"" + fieldname + "== '2'\" /> \n" \
            "<clear namelist=\"" + fieldname + "\" />\n" \
            "<goto nextitem=\"" + fieldname + "\" /> \n" \
            "<elseif cond=\"" + fieldname + "== '3'\" /> \n" \
            "<assign name=\"fromfolder\" expr=\"'" + m_category + "'\" /> \n" \
            "<if cond=\"fromfolder== 'saved'\">\n" \
            "<prompt> \n" \
            "<audio src=\"" + promptUrl + "invalid_entry_try_again.wav\" /> \n" \
            "</prompt> \n" \
            "<assign name=\"playmsg\" expr=\"'no'\" />\n" \
            "<goto nextitem=\"" + fieldname + "\" /> \n" \
            "<else />\n" \
            "<if cond=\"fromfolder=='deleted'\">\n" \
            "<assign name=\"tofolder\" expr=\"'inbox'\" /> \n" \
            "<else />\n" \
            "<assign name=\"tofolder\" expr=\"'saved'\" /> \n" \
            "</if> \n" \
            "<assign name=\"messageids\" expr=\"'" + messageId + "'\" /> \n" \
            "<assign name=\"action\" expr=\"'movemsg'\" /> \n" \
            "<assign name=\"nextfield\" expr=\"'" + nextblock + "'\" /> \n" \
            "<goto nextitem=\"move_msg\" /> \n" \
            "</if>\n" \
            "<elseif cond=\"" + fieldname + "== '4'\" /> \n" \
            "<assign name=\"fromfolder\" expr=\"'" + m_category + "'\" /> \n" \
            "<assign name=\"messageids\" expr=\"'" + messageId + "'\" /> \n" \
            "<assign name=\"nextfield\" expr=\"'" + nextblock + "'\" /> \n" \
            "<if cond=\"fromfolder=='deleted'\"> \n" \
            "<assign name=\"action\" expr=\"'recycledeleted'\" /> \n" \
            "<goto nextitem=\"recycle_deleted_msg\" /> \n" +
            "<else /> \n" \
            "<assign name=\"action\" expr=\"'movemsg'\" /> \n" \
            "<assign name=\"tofolder\" expr=\"'deleted'\" /> \n" \
            "<goto nextitem=\"move_msg\" /> \n" \
            "</if> \n" \
            "<elseif cond=\"" + fieldname + "== '5'\" /> \n" \
            "<assign name=\"playmsg\" expr=\"'no'\" />\n" \
            "<assign name=\"messageids\" expr=\"'" + messageId + "'\" /> \n" \
            "<assign name=\"fromfolder\" expr=\"'" + m_category + "'\" /> \n" \
            "<assign name=\"nextfield\" expr=\"'" + fieldname + "'\" /> \n" \
            "<goto nextitem=\"forwardmsg\" /> \n" \
            "<elseif cond=\"" + fieldname + "== '6'\" /> \n" \
            "<assign name=\"playmsg\" expr=\"'no'\" />\n" \
            "<assign name=\"nextfield\" expr=\"'" + fieldname + "'\" /> \n" \
            "<clear namelist=\"" + fieldname + "\" />\n";

         if( callerIsSipxUser == OS_SUCCESS && voicemailEnabled )
         {
            // Caller is an internal SIPxchange user with voicemail.
            // Let the user proceed with replying to voicemail
            dynamicVxml +=      "<assign name=\"replytovmmailbox\" expr=\"'" + fromMailboxIdentity + "'\"/>\n" \
               "<goto nextitem=\"reply_to_voicemail\" /> \n";
         }
         else if( callerIsSipxUser == OS_SUCCESS )
         {
            // Caller is an internal SIPxchange user without voicemail
            // Reply to voicemail cannot be used and hence play out an error
            dynamicVxml +=      "<prompt> \n" \
               "<audio src=\"" + promptUrl + "no_vm_inbox.wav\" /> \n" \
               "</prompt> \n" \
               "<goto nextitem=\"" + fieldname + "\" /> \n";
         }
         else
         {
            // Outside caller. Reply to voicemail option is not even presented to the user
            dynamicVxml +=      "<prompt> \n" \
               "<audio src=\"" + promptUrl + "invalid_entry_try_again.wav\" /> \n" \
               "</prompt> \n" \
               "<assign name=\"playmsg\" expr=\"'no'\" />\n" \
               "<goto nextitem=\"" + fieldname + "\" /> \n";
         }

         dynamicVxml += "<elseif cond=\"" + fieldname + "== '#'\" />"\
            "<goto nextitem=\""  + nextblock + "\" /> \n" \
            "<elseif cond=\"" + fieldname + "== '*'\" />"\
            "<goto nextitem=\"cancelled\" /> \n" \
            "<else />" \
            "<prompt> \n" \
            "<audio src=\"" + promptUrl + "invalid_entry_try_again.wav\" /> \n" \
            "</prompt> \n" \
            "<assign name=\"playmsg\" expr=\"'no'\" />\n" \
            "<clear namelist=\"" + fieldname + "\" />\n" \
            "<goto nextitem=\"" + fieldname + "\" /> \n" \
            "</if> \n" \
            "</filled> \n" \
            "<noinput count=\"1\"> \n" \
            "<assign name=\"playmsg\" expr=\"'no'\" />\n" \
            "<reprompt /> \n" \
            "</noinput> \n" \
            "<noinput count=\"2\"> \n" \
            "<assign name=\"playmsg\" expr=\"'no'\" />\n" \
            "<reprompt /> \n" \
            "</noinput> \n" \
            "<noinput count=\"3\"> \n" \
            "<prompt bargein=\"false\"> \n" \
            "<audio src=\"" + promptUrl + "thankyou_goodbye.wav\" /> \n" \
            "</prompt> \n" \
            "<disconnect/>\n" \
            "</noinput> \n" \
            "</field>\n";
         iterator++;
      }
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "PlayMessagesCGI::handleOpenVXIRequest: exited loop, iterator = %d",
                    iterator);

      if ( 1 )                  // Always.
      {
         // Convert the iterator into string.
         // Result will be stored in iteratorStr
         // String for storing the iterator
         char iteratorStr1[5];
         sprintf ( iteratorStr1, "%d", iterator );
         dynamicVxml +=  UtlString("<block name=\"blck") + iteratorStr1 + "\">\n" \
            "<var name=\"endofmessages\" expr=\"'";
         if( endOfMessages )
            dynamicVxml +=  "yes'\"/>";
         else
            dynamicVxml +=  "no'\"/>";

         dynamicVxml +=      "<var name=\"nextblockhandle\" expr=\"'" + messageId + "'\"/>";

         dynamicVxml +=      "<return namelist=\"endofmessages nextblockhandle messageidlist\"/>" \
            "</block>\n";
      }

      // Subroutine for moving messages (save and delete operations)
      dynamicVxml +=  "<subdialog name=\"move_msg\" src=\"mediaserver.cgi\"" \
         " namelist=\"action mailbox fromfolder tofolder messageids maintainstatus\"> \n" \
         "<filled> \n" \
         "<if cond=\"move_msg.result == 'success'\" >\n" \
         "<if cond=\"tofolder == 'saved'\" >\n" \
         "<prompt>" \
         "<audio src=\"" + promptUrl + "msg_saved.wav\" />\n " \
         "</prompt>" \
         "<elseif cond=\"tofolder == 'inbox'\" />\n" \
         "<prompt>" \
         "<audio src=\"" + promptUrl + "msg_restored.wav\" />\n " \
         "</prompt>" \
         "<else />\n" \
         "<prompt>"\
         "<audio src=\"" + promptUrl + "msg_deleted.wav\" />\n "\
         "</prompt>"\
         "</if>\n" \
         "<else />\n" \
         "<prompt>"\
         "<audio src=\"" + promptUrl + "msgnotsaved.wav\" />\n "\
         "</prompt>"\
         "</if>\n" \
         "<goto expritem=\"nextfield\" />"\
         "</filled>\n" \
         "</subdialog>\n";

      // Subroutine for purging deleted messages

      dynamicVxml +=  "<subdialog name=\"recycle_deleted_msg\" src=\"mediaserver.cgi\"" \
         " namelist=\"action mailbox messageids\"> \n" \
         "<filled> \n" \
         "<if cond=\"recycle_deleted_msg.result == 'success'\" >\n" \
         "<prompt>" \
         "<audio src=\"" + promptUrl + "msg_deleted_permanently.wav\" />\n " \
         "</prompt>" \
         "</if>\n" \
         "<goto expritem=\"nextfield\" />"\
         "</filled>\n" \
         "</subdialog>\n";

      // Subroutine for forwarding messages.
      dynamicVxml +=  "<subdialog name=\"forwardmsg\" src=\"" + mediaserverUrl + "/vm_vxml/forwardmessage.vxml\">" \
         "<param name=\"mailbox\" expr=\"mailbox\" />\n" \
         "<param name=\"mediaserverurl\" expr=\"'" + ivrPromptUrl + "'\" />\n" \
         "<param name=\"securemediaserverurl\" expr=\"'" + secureMediaserverUrl + "'\"/>\n" \
         "<param name=\"fromfolder\" expr=\"fromfolder\" />\n" \
         "<param name=\"messageids\" expr=\"messageids\" />\n" \
         "<param name=\"messageidlist\" expr=\"messageidlist\" />\n" \
         "<param name=\"category\" expr=\"'" + m_category + "'\" />\n" \
         "<filled> \n" \
         "<goto expritem=\"nextfield\" />"\
         "</filled>\n" \
         "</subdialog>\n";

      // Code for returning the call.
      dynamicVxml += "<transfer name=\"return_call\" destexpr=\"transferto\" />";

      // Code to handle disconnect event
      dynamicVxml += "<catch event=\"telephone.disconnect.hangup\">\n" \
         "<assign name=\"action\" expr=\"'updatestatus'\"/>\n" \
         "<assign name=\"category\" expr=\"'" + m_category + "'\" />\n" \
         "<if cond=\"category=='heard'\"> \n" \
         "<assign name=\"messageidlist\" expr=\"'" + m_unheardMsgIdList + ";'\" /> \n" \
         "</if> \n"\
         "<goto nextitem=\"updatestatus\" />\n" \
         "</catch>";

      // Subdialog to update the message status
      dynamicVxml += "<subdialog name=\"updatestatus\" src=\"mediaserver.cgi\" namelist=\"action mailbox category messageidlist\" >\n" \
         "<filled>\n" \
         "<if cond=\"updatestatuscalledfrom == 'transfer'\"> \n" \
         "<goto nextitem=\"return_call\" /> \n" \
         "<else /> \n" \
         "<exit/>\n" \
         "</if> \n" \
         "</filled>\n" \
         "</subdialog>";

      // Subroutine for "reply to voicemail" feature
      dynamicVxml +=  "<subdialog name=\"reply_to_voicemail\" src=\"" + mediaserverUrl + "/vm_vxml/replytovoicemail.vxml\">" \
         "<param name=\"mailbox\" expr=\"replytovmmailbox\" />\n" \
         "<param name=\"from\" expr=\"mailbox\" />\n" \
         "<param name=\"mediaserverurl\" expr=\"'" + ivrPromptUrl + "'\" />\n" \
         "<param name=\"securemediaserverurl\" expr=\"'" + secureMediaserverUrl + "'\"/>\n" \
         "<param name=\"messageidlist\" expr=\"messageidlist\" />\n" \
         "<param name=\"category\" expr=\"'" + m_category + "'\" />\n" \
         "<filled> \n" \
         "<goto expritem=\"nextfield\" />"\
         "</filled>\n" \
         "</subdialog>\n";

      dynamicVxml += "<block name=\"cancelled\">\n" \
         "<var name=\"endofmessages\" expr=\"'cancelled'\"/>" \
         "<assign name=\"category\" expr=\"'" + m_category + "'\" />\n" \
         "<return namelist=\"endofmessages messageidlist\"/>" \
         "</block>\n";

      dynamicVxml +=  "</form>";
   }
   else
   {
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "PlayMessagesCGI::handleOpenVXIRequest: getMessageBlock returns %d",
                    endOfMessages);

      // Messages are not available for the given category and lastMessageHandle.
      dynamicVxml +=  "<form><block>" \
         "<var name=\"endofmessages\" expr=\"'nomessages'\" />";
      if( m_category == "heard" )
         dynamicVxml += "<var name=\"messageidlist\" expr=\"'" + m_unheardMsgIdList + ";'\" /> \n";
      else
         dynamicVxml += "<var name=\"messageidlist\" expr=\"'0'\" /> \n";

      dynamicVxml +=  "<return namelist=\"endofmessages messageidlist\"/>" \
         "</block></form>";
   }

   dynamicVxml += VXML_END;

   // Write out the dynamic VXML script to be processed by OpenVXI
   if (out)
   {
      out->remove(0);
      UtlString responseHeaders;
      MailboxManager::getResponseHeaders(dynamicVxml.length(), responseHeaders);

      out->append(responseHeaders.data());
      out->append(dynamicVxml.data());
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "PlayMessagesCGI::handleOpenVXIRequest: wrote VXML to 'out'");
   }
   return OS_SUCCESS;
}

UtlString
PlayMessagesCGI::getTimestampVxml(const UtlString& timestamp, const UtlString& promptUrl) const
{
   UtlString vxmlTimestamp = "<prompt cond=\"playmsg=='yes'\"> \n" \
      "<audio src=\"" + promptUrl + "msg_received.wav" + "\" /> \n" \
      "</prompt>\n";
   UtlString snippet = "";

   char  w[8], w1[8];
   char  m[8], m1[8];
   char  y[8], y1[8];
   char  tm[8], tm1[8], sec[8];
   char  am[8], am1[8];
   char  zn[8], zn1[8];
   int   day, day1;

   memset(w, 0, 8*sizeof(char));
   memset(m, 0, 8*sizeof(char));
   memset(y, 0, 8*sizeof(char));
   memset(tm, 0, 8*sizeof(char));
   memset(am, 0, 8*sizeof(char));
   memset(zn, 0, 8*sizeof(char));
   memset(w1, 0, 8*sizeof(char));
   memset(m1, 0, 8*sizeof(char));
   memset(y1, 0, 8*sizeof(char));
   memset(tm1, 0, 8*sizeof(char));
   memset(am1, 0, 8*sizeof(char));
   memset(zn1, 0, 8*sizeof(char));

   // timestamp is in the format "Mon, 26-Sep-2002 07:21:32 PM EST"
   sscanf(timestamp.data(), "%s %d-%3c-%s %5c %s %s %s", w, &day, m, y, tm, sec, am, zn);

   UtlString   strTime;
   OsDateTime::getLocalTimeString(strTime);
   sscanf(strTime.data(), "%s %d-%3c-%s %5c %s %s %s", w1, &day1, m1, y1, tm1, sec, am1, zn1);

   w[3] = w1[3] = 0;
   int matched = 0;
   if (strcmp(y, y1) == 0)
   {
      if (strcmp(m, m1) == 0)
      {
         if (day == day1)
         {
            vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"> \n" \
               "<audio src=\"" + promptUrl + "today.wav" + "\" /> \n" \
               "</prompt>\n";
            matched = 1;
         }
         else if (day == (day1 - 1))
         {
            vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"> \n" \
               "<audio src=\"" + promptUrl + "yesterday.wav" + "\" /> \n" \
               "</prompt>\n";
            matched = 1;
         }
         else if (day1 < (day + 7))
         {
            // We have retrieved "Mon,"
            vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"> \n" \
               "<audio src=\"" + promptUrl + "on.wav" + "\" /> \n" \
               "</prompt>\n";
            vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"><say-as type=\"date:w\">" + UtlString(w) + "</say-as></prompt>\n";
            matched = 1;
         }
      }
      if (!matched)
      {
         matched = 2; // same year
      }
   }

   UtlString str(m);
   if (matched != 1)
   {
      if( str.compareTo("jan", UtlString::ignoreCase) == 0 )
         str = "01";
      else if( str.compareTo("feb", UtlString::ignoreCase) == 0 )
         str = "02";
      else if( str.compareTo("mar", UtlString::ignoreCase) == 0 )
         str = "03";
      else if( str.compareTo("apr", UtlString::ignoreCase) == 0 )
         str = "04";
      else if( str.compareTo("may", UtlString::ignoreCase) == 0 )
         str = "05";
      else if( str.compareTo("jun", UtlString::ignoreCase) == 0 )
         str = "06";
      else if( str.compareTo("jul", UtlString::ignoreCase) == 0 )
         str = "07";
      else if( str.compareTo("aug", UtlString::ignoreCase) == 0 )
         str = "08";
      else if( str.compareTo("sep", UtlString::ignoreCase) == 0 )
         str = "09";
      else if( str.compareTo("oct", UtlString::ignoreCase) == 0 )
         str = "10";
      else if( str.compareTo("nov", UtlString::ignoreCase) == 0 )
         str = "11";
      else if( str.compareTo("dec", UtlString::ignoreCase) == 0 )
         str = "12";

      char date[3] = {0,0,0};
      sprintf(date, "%d", day);

      if (matched == 2)
      {
         str = str + "/" + date;
         vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"> \n" \
            "<audio src=\"" + promptUrl + "on.wav" + "\" /> \n" \
            "</prompt>\n";
         vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"><say-as type=\"date:w\">" + UtlString(w) + "</say-as></prompt>\n";
         vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"><say-as type=\"date:md\">" + str + "</say-as></prompt>\n";
      }
      else
      {
         str = str + "/" + date + "/" + y;
         vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"> \n" \
            "<audio src=\"" + promptUrl + "on.wav" + "\" /> \n" \
            "</prompt>\n";
         vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"><say-as type=\"date:w\">" + UtlString(w) + "</say-as></prompt>\n";
         vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"><say-as type=\"date:mdy\">" + str + "</say-as></prompt>\n";
      }
   }

   // We have retrieved "07:21:32"
   vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"><audio src=\"" + promptUrl + "at.wav\"/></prompt>\n";
   vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"><say-as type=\"time:hm\">" + UtlString(tm) + "</say-as></prompt>\n";

   str = UtlString(am);
   str.toLower();

   // We have retrieved "PM"
   vxmlTimestamp += "<prompt cond=\"playmsg=='yes'\"><audio src=\"" + promptUrl + str + ".wav\"/></prompt>\n";

   return vxmlTimestamp;
}

OsStatus
PlayMessagesCGI::getFolderNamesSnippet( UtlString& rFolderSnippet ) const
{
   UtlSortedList folderList;
   MailboxManager* pMailboxManager = MailboxManager::getInstance();
   pMailboxManager->getMailboxFolders( m_mailboxIdentity, folderList );

   rFolderSnippet = "<tr> \n" \
      "<td height=\"20\" valign=\"top\" nowrap> \n" \
      "<input type=\"hidden\" name=\"fromfolder\" value=\"" + m_category + "\"/>\n" \
      "<input type=\"hidden\" name=\"maintainstatus\" value=\"yes\"/>\n" \
      "<input type=\"hidden\" name=\"fromweb\" value=\"yes\"/>\n" \
      "<input type=\"hidden\" name=\"nextblockhandle\" value=\"" + m_strNextBlockHandle + "\">\n" \
      "<select name=\"tofolder\" size=\"1\">\n" \
      "<option value=\"1\" selected>--- Move to Folder ---</option>\n";

   while ( folderList.entries() > 0 )
   {
      UtlString* rwFolderName =
         (UtlString*) folderList.removeAt(0);

      UtlString folderName = rwFolderName->data();
      delete rwFolderName;

      // If you are displaying 'inbox' folder's contents,
      // that is, m_category = 'inbox', then it does not make sense to
      // list it in the move to folder option.

      // Also, as we already have the 'Delete' button,
      // there is no need for listing 'deleted' in the move to folder option.
      if( folderName != m_category && folderName != "deleted")
      {
         // Initialize the display name to the folder name
         UtlString folderDisplayName( folderName );

         if( folderName == "inbox" )
         {
            folderDisplayName = INBOX_DISPLAY_NAME;
         } else if( folderName == "saved" )
         {
            folderDisplayName = SAVED_DISPLAY_NAME;
         }

         rFolderSnippet += "<option value=\"" + folderName + "\">" + folderDisplayName + "</option>\n";
      }
   }

   rFolderSnippet += "</select>\n" \
      "&nbsp; \n" \
      "<input type=\"submit\" name=\"action\" value=\"Move\">\n";


   if( m_category == "deleted" )
   {
      rFolderSnippet +=   "<input type=\"submit\" name=\"action\" value=\"Delete\" onClick=\"return confirm('" + UtlString(DELETE_MSG_CONFIRMATION) + "');\">\n"\
         "&nbsp;<input type=\"submit\" name=\"action\" value=\"Empty Folder\" onClick=\"return confirm('" + UtlString(DELETE_MSG_CONFIRMATION) + "');\">\n" \
         "<input type=\"hidden\" name=\"deletemode\" value=\"purge\">\n";
   }
   else
   {
      rFolderSnippet += "<input type=\"submit\" name=\"action\" value=\"Delete\">\n" \
         "<input type=\"hidden\" name=\"deletemode\" value=\"delete\">\n";
   }

   rFolderSnippet +=   "&nbsp;<input type=\"button\" value=\"Select All\" onClick=\"javascript:CheckAll();\">\n" \
      "<input type=\"button\" value=\"Clear All\" onClick=\"javascript:ClearAll();\">\n" \
      "</td>\n";

   UtlString msgLinks;
   displayAdditionalMsgLinks( msgLinks );
   rFolderSnippet += msgLinks + "</tr>\n";

   return OS_SUCCESS;
}

OsStatus
PlayMessagesCGI::getPageTitle( UtlString& rTitle  ) const
{
   // Get the display name for this folder.
   UtlString folderDisplayName;
   rTitle = "";
   if( m_category == "inbox" || m_category == "unheard" || m_category == "heard" )
   {
      folderDisplayName = INBOX_DISPLAY_NAME;
   }
   else if( m_category == "saved" )
   {
      folderDisplayName = SAVED_DISPLAY_NAME;
   }
   else if( m_category == "deleted" )
   {
      folderDisplayName = DELETED_DISPLAY_NAME;
   }
   else
   {
      folderDisplayName = m_category;
      rTitle = "Personal Folders - ";
   }

   rTitle += "Contents of <i>" + folderDisplayName + "</i>";

   return OS_SUCCESS;
}

OsStatus
PlayMessagesCGI::formatMessageDuration( const UtlString& unformattedDuration,
                                        UtlString& rFormattedDuration ) const
{
   UtlString durationsecs = unformattedDuration;
   int iDuration = atoi( durationsecs.data() );
   char temp[10];
   if( iDuration >= 3600 )
   {
      // Retrieve the hour.
      int hours = iDuration / 3600;
      iDuration = iDuration - (hours * 3600);
      sprintf( temp, "%02d", hours );
      durationsecs = UtlString( temp ) + ":";
   }
   else
   {
      durationsecs = "00:";
   }

   if( iDuration >= 60 )
   {
      // Retrieve the hour.
      int mins = iDuration / 60;
      iDuration = iDuration - (mins * 60);
      sprintf( temp, "%02d", mins );

      durationsecs += UtlString( temp ) + ":";
   }
   else
   {
      durationsecs += "00:";
   }

   sprintf( temp, "%02d", iDuration );

   durationsecs += temp;

   rFormattedDuration = durationsecs;

   return OS_SUCCESS;
}

OsStatus
PlayMessagesCGI::formatTimestamp(   const UtlString& unformattedTimestamp,
                                    UtlString& rFormattedTimestamp ) const
{
   UtlString strTimestamp = unformattedTimestamp;

   // change hh:mm:ss to hh:mm
   unsigned int i = strTimestamp.last(':');
   if( i != UTL_NOT_FOUND )
      strTimestamp = strTimestamp.remove(i, 3 );

   // Change 12-Sep-2002 in timestamp to 12-Sep-02
   i = strTimestamp.last('-');
   if( i != UTL_NOT_FOUND )
      strTimestamp = strTimestamp.remove(i+1, 2 );

   rFormattedTimestamp = strTimestamp;

   return OS_SUCCESS;
}

OsStatus
PlayMessagesCGI::formatFromInfo(    const UtlString& unformattedFromInfo,
                                    UtlString& rFormattedFromInfo,
                                    const UtlBoolean& firstentry) const
{
   UtlString fromString = unformattedFromInfo;
   Url fromUrl = fromString.data();

   UtlString displayName;
   fromUrl.getDisplayName(displayName);

   if( !displayName.isNull() )
      HttpMessage::unescape( displayName );

   UtlString fromIdentity;
   fromUrl.getIdentity(fromIdentity);

   UtlString indent = "<img src=\"/images/spacer.gif\" width=\"15\" height=\"1\" border=\"0\">";
   if( !firstentry )
   {
      if( !displayName.isNull() )
         displayName = indent + displayName;

      fromIdentity = indent + fromIdentity;
   }

   if (!displayName.isNull())
      fromString = displayName +  "<br>" + fromIdentity;
   else
      fromString = fromIdentity;

   rFormattedFromInfo = fromString;

   return OS_SUCCESS;
}

OsStatus
PlayMessagesCGI::displayAdditionalMsgLinks( UtlString& rMsgLinksSnippet ) const
{
   UtlSortedList svNextBlockHandles;
   MailboxManager* pMailboxManager = MailboxManager::getInstance();
   pMailboxManager->getMessageBlockHandles(    m_mailboxIdentity,
                                               svNextBlockHandles,
                                               m_category,
                                               m_iBlockSize );

   if( svNextBlockHandles.entries() == 0 )
   {
      // All messages in the folder have been displayed.
      rMsgLinksSnippet = "<td height=\"20\" align=\"right\">&nbsp;</td> \n";
   }
   else
   {

      UtlString spacer = "<img src=\"/images/spacer.gif\" width=\"10\" height=\"1\" border=\"0\">";
      UtlString playMsgUrl = "/cgi-bin/voicemail/mediaserver.cgi?action=playmsg" \
         "&category=" + UtlString( m_category ) +
         "&from=web&fromweb=yes" \
         "&nextblockhandle=";

      // Display links for accessing webpages listing more messages in that folder.
      rMsgLinksSnippet = "<td height=\"20\" align=\"right\" valign=\"top\" width=\"250\"> \n" \
         "More Messages" + spacer + ":" + spacer;

      if( m_iNextBlockHandle == -1 )
         rMsgLinksSnippet += "1" + spacer + "\n";
      else
         rMsgLinksSnippet += "<a href=\"" + playMsgUrl + "-1\">1</a>" + spacer + "\n";

      // Get the other message handles from the vector.
      int pageid = 2;
      while( svNextBlockHandles.entries() > 0 )
      {
         // Note that for now, we are displaying the messages from latest to oldest.

         UtlString* rwfilename =
            (UtlString*) svNextBlockHandles.removeAt(svNextBlockHandles.entries() -1);

         UtlString filename = rwfilename->data();
         int nextBlockHandle = atoi( filename.data() );

         char temp[5];
         sprintf(temp, "%d", pageid );
         UtlString strPageId = temp;

         if( m_iNextBlockHandle == nextBlockHandle )
            rMsgLinksSnippet += strPageId + spacer + "\n";
         else
            rMsgLinksSnippet += "<a href=\"" + playMsgUrl + filename + "\">" + strPageId + "</a>" + spacer + "\n";

         pageid++;
      }
   }

   return OS_SUCCESS;
}

OsStatus
PlayMessagesCGI::getHtmlContent( UtlString& rHtmlOutput,
                                 const UtlString& cgiUrl) const
{
   // Get all the folder names.
   UtlString rFolderSnippet;
   getFolderNamesSnippet( rFolderSnippet );

   // Construct the helper object that will retrieve the message details.
   GetMessageDetailsHelper gmdHelper;
   UtlBoolean endOfMessages;
   OsStatus result = gmdHelper.getMessageBlock( m_mailboxIdentity,
                                                m_category,
                                                m_iBlockSize,
                                                m_iNextBlockHandle,
                                                endOfMessages,
                                                TRUE,
                                                TRUE);
   if ( result == OS_SUCCESS )
   {
      // HTML code for the table header used for displaying message info.
      rHtmlOutput =
         "<tr> \n" \
         "<td colspan=\"2\"> \n" \
         "<table border=\"0\" align=\"left\" width=\"100%\">\n" \
         "<tr>\n" \
         "<td colspan=\"2\" height=\"57\"> \n" \
         "<table class=\"bglist\" cellspacing=\"1\" cellpadding=\"4\" border=\"0\" width=\"100%\">\n" \
         "<tr> \n" \
         "<th>&nbsp;</th>\n" \
         "<th>Sender</th>\n" \
         "<th>Subject</th>\n" \
         "<th>Duration</th>\n" \
         "<th>Date</th>\n" \
         "<th>Play</th>\n" \
         "<th>Edit</th>\n" \
         "</tr>\n";

      // Get information about each message retrieved.
      while (gmdHelper.getNextMessageCollectable() == OS_SUCCESS )
      {
         UtlString unheardMessageIndicatorStart, unheardMessageIndicatorEnd;

         // Indicates if this is the first message in the list of forwarded messages.
         UtlBoolean firstentry = TRUE;
         UtlBoolean msgHeard = FALSE;

         // Holds the depth of forwarded messages.
         int numForwardedAttachments;
         gmdHelper.getMessageAttachmentsCount( numForwardedAttachments );

         // For forwarded messages, each component of the forwarded message
         // is displayed on a separate row.
         // However, there is just one checkbox, play link and edit link.
         // The number of rows these three columns span depends on the number of
         // forwarded messages.
         UtlString rowspan = "";
         if( numForwardedAttachments > 1 )
         {
            char temp[10];
            sprintf(temp, "%d", numForwardedAttachments );
            rowspan = " rowspan=\"" + UtlString( temp ) + "\" ";
         }

         // this is where we create the list of messages to play back
         while( gmdHelper.getNextMessage() == OS_SUCCESS )
         {
            if( firstentry )
            {
               // Unheard messages should be displayed in bold.
               // Check the message status and add HTML code appropriately
               UtlString msgheardstatus =
                  gmdHelper.getMessageDetails( UtlString("status") );

               if( msgheardstatus == "unheard" )
               {
                  unheardMessageIndicatorStart = "<b>";
                  unheardMessageIndicatorEnd = "</b>";
                  msgHeard = FALSE;
               }
               else
               {
                  unheardMessageIndicatorStart = "";
                  unheardMessageIndicatorEnd = "";
                  msgHeard = TRUE;
               }
            }

            // Format the data displayed in the 'from' field
            UtlString fromString = gmdHelper.getMessageDetails( UtlString("from") );
            UtlString formattedFrom;
            formatFromInfo( fromString, formattedFrom, firstentry );
            UtlString sender = unheardMessageIndicatorStart + formattedFrom + unheardMessageIndicatorEnd;


            // Message subject
            UtlString subject =  unheardMessageIndicatorStart +
               gmdHelper.getMessageDetails( UtlString("subject") ) +
               unheardMessageIndicatorEnd;


            // Display duration as hh:mm:ss. This is stored in xml as seconds.
            UtlString durationsecs = gmdHelper.getMessageDetails( UtlString("durationsecs") );
            UtlString formattedDuration;
            formatMessageDuration(durationsecs, formattedDuration);
            UtlString duration = unheardMessageIndicatorStart + formattedDuration + unheardMessageIndicatorEnd;


            // Change timestamp from 12-Sep-2002 hh:mm:ss to 12-Sep-02 hh:mm
            UtlString strTimestamp = gmdHelper.getMessageDetails( UtlString("timestamp") );
            UtlString formattedTimestamp;
            formatTimestamp( strTimestamp, formattedTimestamp );
            UtlString timestamp = unheardMessageIndicatorStart + formattedTimestamp + unheardMessageIndicatorEnd;


            if( firstentry )
            {

               // Get the name of the WAV file to be linked to the speaker button.
               UtlString msgUrl = gmdHelper.getMessageDetails( UtlString("playlisturl") );
               int startIndex = msgUrl.last('/') + 1;
               // Extract the eight-digit message number by deducting the length of the
               // "-dd.wav" at the end.
               int length = msgUrl.length() - startIndex - 7;

               // URL for playing the message.
               UtlString playUrl;
               if( msgHeard )
               {
                  // Provide a link to the WAV file.
                  // Construct the string to appear inside HREF="...",
                  // It is actually a URL, followed by an ONCLICK
                  // attribute, with quotes added to make it work.
                  // This uses a trick -- if Javascript is enabled, when
                  // the user clicks on the link, the browser
                  // calls playMsgJs() to play the message, then the ONCLICK returns
                  // false to inhibit the browser from any further action.
                  // But if Javascript is not enabled, the browser follows
                  // the HREF to the WAV file and plays it however it
                  // chooses.
                  playUrl = msgUrl + "\" onclick=\"playMsgJs('" + msgUrl + "'); return false";
               }
               else
               {
                  // This link uses the same tricks as the previous one,
                  // but it is more complicated.
                  // If Javascript is off, we just provide a link to the WAV file.
                  // But if Javascript is on, we want to do three things:
                  // 1. Send an HTTP request to the CGI that marks the message as heard.
                  // We do this by loading the hiddenFrame from a CGI URL.
                  // 2. Refresh the message listing, so that the
                  // visible status of the message is updated.
                  // We do this by setting location.reload() to execute in 1.000 seconds.
                  // 3. Play the message.
                  // We do this by calling playMsgJs().
                  playUrl = msgUrl + "\" onclick=\"top.frames['hiddenFrame'].location.href='" + cgiUrl +
                     "?action=updatestatus&fromweb=yes"
                     "&category=" + m_category +
                     "&messageidlist=" + msgUrl(startIndex, length) + "'; "
                     "setTimeout('location.reload()', 1000); "
                     "playMsgJs('" + msgUrl + "'); "
                     "return false";
               }

               // Construct the A element that plays the message.
               UtlString play = "<a href=\"" + playUrl + "\" target=\"hiddenFrame\"><img src=\"/images/spkr.gif\" width=\"20\" height=\"18\" border=\"0\" alt=\"Play message on PC speaker\"></a>  \n";

               UtlString messageid = gmdHelper.getMessageDetails(UtlString("messageid"));
               UtlString editMessageUrl =   cgiUrl +
                  "?action=editmsg" +
                  "&foldername=" + m_category +
                  "&messageid=" + messageid + "-00" +
                  "&subject=" + subject +
                  "&nextblockhandle=" + m_strNextBlockHandle +
                  "&formsubmitted=no" +
                  "&fromweb=yes";

               UtlString edit    = "<a href=\"" + editMessageUrl + "\"><img src=\"/images/editicon.gif\" width=\"12\" height=\"12\" border=\"0\"></a>";

               rHtmlOutput +=
                  "<tr> \n" \
                  "<td" + rowspan + ">  \n" \
                  "<input type=\"checkbox\" name=\"" + messageid + "\" > \n" \
                  "</td> \n" \
                  "<td nowrap>" + sender + "</td> \n" \
                  "<td align=\"left\">" + subject + "</td> \n" \
                  "<td align=\"left\">" + duration + "</td> \n" \
                  "<td align=\"left\" nowrap>" + timestamp + "</td> \n" \
                  "<td align=\"center\"" + rowspan + ">" + play + "</td> \n" \
                  "<td align=\"center\"" + rowspan + ">" + edit + "</td> \n" \
                  "</tr> \n";
               firstentry = FALSE;
            }
            else
            {
               rHtmlOutput +=
                  "<tr> \n " \
                  "<td>" + sender + "</td> \n" \
                  "<td align=\"left\">" + subject + "</td> \n" \
                  "<td align=\"left\">" + duration + "</td> \n" \
                  "<td align=\"left\">" + timestamp + "</td> \n" \
                  "</tr> \n";
            }
         }
      }

      rHtmlOutput +=
         "</table> \n" \
         "</td> \n" \
         "</tr> \n" + rFolderSnippet;
      /*"<td> \n" \
        "<div align=\"right\"> \n" \
        "<input type=\"hidden\" name=\"folder\" value=\"" + m_category + "\"> \n" \
        "<input type=\"submit\" name=\"Submit\" value=\"Forward...\"> \n" \
        "</div> \n" \
        "</td> \n" \
        "</tr> \n";*/
   }

   return result;
}


OsStatus
PlayMessagesCGI::validateFromUrl(const UtlString& fromStr,
                                 UtlBoolean& rVoicemailEnabled,
                                 UtlString& rFromExtension,
                                 UtlString& rFromMailboxIdentity) const
{
   // Get the identity of the caller (fromURL)
   Url callerUrl( fromStr );
   UtlString fromIdentity;
   callerUrl.getIdentity(fromIdentity);

   // construct the helper object for validating the from URL
   ValidateMailboxCGIHelper validateFromUrlHelper ( fromIdentity );

   // check for voicemail permission only after
   // confirming that the caller is an internal user
   UtlBoolean checkPermission = FALSE;

   // check to see if the caller is an internal sipxchange user
   OsStatus result = validateFromUrlHelper.validate( checkPermission );
   if( result == OS_SUCCESS )
   {
      // retrieve extension and mailbox identity
      validateFromUrlHelper.getExtension( rFromExtension );
      validateFromUrlHelper.getMailboxIdentity( rFromMailboxIdentity );

      // TBD - This code could be optimized by writing a function that
      // merely checks the permission for a given identity.
      // Currently it checks the credential db to confirm that the identity
      // is valid before looking in the Permissions DB.
      ValidateMailboxCGIHelper checkPermissionHelper ( rFromMailboxIdentity );
      checkPermission = TRUE;
      // check to see if the caller has voicemail inbox
      OsStatus voicemailStatus = checkPermissionHelper.validate( checkPermission );

      if( voicemailStatus == OS_SUCCESS )
         rVoicemailEnabled = TRUE;
      else
         rVoicemailEnabled = FALSE;
   }
   else
   {
      rFromExtension = "";
      rFromMailboxIdentity = "";
      rVoicemailEnabled = FALSE;
   }

   return result;
}

UtlString
PlayMessagesCGI::getMsgOriginVxml ( const OsStatus& callerIsSipxUser,
                                    const UtlBoolean& voicemailEnabled,
                                    const UtlString& fromExtension,
                                    const UtlString& fromMailboxIdentity,
                                    const UtlString& unformattedFrom,
                                    const UtlString& promptUrl) const
{
   // Holds the VXML snippet for playing message origin information
   UtlString msgOriginNotificationVxml = "<prompt cond=\"playmsg=='yes'\"> \n" \
      "<audio src=\"" + promptUrl + "from.wav" + "\" /> \n" \
      "</prompt> \n";


   // check to see if the caller was an internal sipxchange user
   if( callerIsSipxUser == OS_SUCCESS )
   {
      // check to see if the caller has voicemail enabled
      if( voicemailEnabled )
      {
         // check if the caller has recorded his name
         ActiveGreetingHelper greetingHelper;
         UtlString greetingUrl;
         if( greetingHelper.getRecordedName(fromMailboxIdentity, greetingUrl, FALSE ) == OS_SUCCESS )
         {
            // play the recorded name
            msgOriginNotificationVxml +="<prompt cond=\"playmsg=='yes'\"> \n" \
               "<audio src=\"" + greetingUrl + "\" /> \n" \
               "<audio src=\"" + promptUrl + "at.wav" + "\" /> \n" \
               "</prompt> \n";
         }
      }

      // check if the caller has an extension
      if( !fromExtension.isNull() && fromExtension.length() > 0 )
      {
         // Check if the extension is a URL
         int index = fromExtension.index("@");
         UtlString extensionUserId;
         if( index >= 0 )
         {
            // construct a URL object
            Url extensionUrl ( fromExtension );
            extensionUrl.getUserId ( extensionUserId );
         }
         else
         {
            extensionUserId = fromExtension;
         }


         // Spell out the extension of the caller
         msgOriginNotificationVxml +="<prompt cond=\"playmsg=='yes'\"> \n" \
            "<audio src=\"" + promptUrl + "extension.wav" + "\" /> \n" \
            "</prompt>" \
            "<prompt cond=\"playmsg=='yes'\"> \n" \
            "<say-as type=\"digits\">" + extensionUserId + "</say-as> \n" \
            "</prompt>";
      }
      else
      {
         // spell out the user id
         Url urlFromMailboxIdentity (fromMailboxIdentity);
         UtlString fromUserId;
         urlFromMailboxIdentity.getUserId( fromUserId );

         msgOriginNotificationVxml +="<prompt cond=\"playmsg=='yes'\"> \n" \
            "<audio src=\"" + promptUrl + "extension.wav" + "\" /> \n" \
            "</prompt>" \
            "<prompt cond=\"playmsg=='yes'\"> \n" \
            "<say-as type=\"acronym\">" + fromUserId + "</say-as> \n" \
            "</prompt>";
      }
   }
   else
   {
      // check if E.164 number can be extracted for the outside caller
      Url fromUrl (unformattedFrom);
      UtlString fromUserId;
      fromUrl.getUserId( fromUserId );

      UtlBoolean allDigits = FALSE;
      int len = fromUserId.length();
      if( !fromUserId.isNull() && len > 0 )
      {
         // Must be all digits
         const char *number = fromUserId.data();
         for (int i = 0; i < len; i++)
         {
            allDigits = isdigit(number[i]);
            if (!allDigits) break;
         }

         if (allDigits)
         {
            // play the E164 number
            msgOriginNotificationVxml +="<prompt cond=\"playmsg=='yes'\"> \n" \
               "<say-as type=\"digits\">" + fromUserId + "</say-as> \n" \
               "</prompt>";
         }
      }

      if (!allDigits)
      {
         // caller id unknown. Simply indicate that the call was from a PSTN number
         msgOriginNotificationVxml += "<prompt cond=\"playmsg=='yes'\"> \n" \
            "<audio src=\"" + promptUrl + "an_outside_caller.wav" + "\" /> \n" \
            "</prompt>";
      }
   }

   return msgOriginNotificationVxml;
}

UtlString
PlayMessagesCGI::getMessageMenuVxml ( const UtlString& promptUrl,
                                      const OsStatus& callerIsSipxUser,
                                      const UtlBoolean& voicemailEnabled) const
{
   // VXML snippet for playing the message menu options
   UtlString messageMenuPrompt = "<prompt> \n" \
      "<audio src=\"" + promptUrl + "to_play_info_about_message.wav\" /> \n" \
      "<audio src=\"" + promptUrl + "press_1.wav\" /> \n" \
      "<audio src=\"" + promptUrl + "to_replay.wav\" /> \n" \
      "<audio src=\"" + promptUrl + "press_2.wav\" /> \n";

   if( m_category.compareTo("deleted", UtlString::ignoreCase) == 0 )
   {
      // Owner is listening to messages in deleted messages folder.
      // Hence, 3 corresponds to option "restore to inbox" and
      // 4 corresponds to "permanently delete"
      messageMenuPrompt += "<audio src=\"" + promptUrl + "to_restore_to_inbox.wav\" /> \n" \
         "<audio src=\"" + promptUrl + "press_3.wav\" /> \n" \
         "<audio src=\"" + promptUrl + "to_delete_permanently.wav\" /> \n" \
         "<audio src=\"" + promptUrl + "press_4.wav\" /> \n";
   }
   else
   {
      if( m_category.compareTo("saved", UtlString::ignoreCase) != 0 )
      {
         // Owner is not listening to saved messages
         // Hence, 3 corresponds to option "to save message"
         messageMenuPrompt += "<audio src=\"" + promptUrl + "to_save.wav\" /> \n" \
            "<audio src=\"" + promptUrl + "press_3.wav\" /> \n";
      }
      // Owner is listening to inbox or saved messages.
      // Hence 4 corresponds to "delete message"
      messageMenuPrompt += "<audio src=\"" + promptUrl + "to_delete.wav\" /> \n" \
         "<audio src=\"" + promptUrl + "press_4.wav\" /> \n";
   }

   // Key 5 corresponds to forward message
   messageMenuPrompt += "<audio src=\"" + promptUrl + "to_forward_this_message.wav\" /> \n" \
      "<audio src=\"" + promptUrl + "press_5.wav\" /> \n";

   if( callerIsSipxUser == OS_SUCCESS )
   {
      // voicemail message to be played was left by an internal sipxchange user.
      // Enable the "reply to voicemail" option
      messageMenuPrompt += "<audio src=\"" + promptUrl + "to_reply.wav\" /> \n" \
         "<audio src=\"" + promptUrl + "press_6.wav\" /> \n";
   }

   messageMenuPrompt += "<audio src=\"" + promptUrl + "to_play_the_next_message.wav\" /> \n" \
      "<audio src=\"" + promptUrl + "press_pound.wav\" /> \n" \
      "<audio src=\"" + promptUrl + "to_return_to_main_menu.wav\" /> \n" \
      "<audio src=\"" + promptUrl + "press_star.wav\" /> \n" \
      "</prompt>";

   return messageMenuPrompt;
}


OsStatus PlayMessagesCGI::handlePlayFromEmailNotification( UtlString* out )
{
   // This request was from user clicking on the "Show Voicemail Inbox"
   // in the email notification.

   UtlString cgiURL, dynamicHTML;
   MailboxManager* pMailboxManager = MailboxManager::getInstance();

   // Get the mediaserver base URL
   OsStatus result = pMailboxManager->getMediaserverURLForWeb( cgiURL );
   if( result == OS_SUCCESS )
   {
      // URL of the mediaserver CGI
      cgiURL += CGI_URL;

      // Construct the output HTML string.
      dynamicHTML =   NO_HTML_CACHE_HEADER \
         "<HTML>\n" \
         "<HEAD>\n" \
         "<TITLE>MediaServer User Inbox</TITLE>\n" \
         "</HEAD>\n" \
         "<frameset rows=\"*,0\">\n" \
         "<frame name=\"mainFrame\" scrolling=\"true\" src=\" " + cgiURL + "?action=playmsg&" \
         "category=" + m_category + "&nextblockhandle=" + m_strNextBlockHandle + "&fromweb=yes\">\n" \
         "<frame name=\"hiddenFrame\" src=\"\">\n "\
         "</frameset>\n" \
         "</html>\n ";
   }
   else
   {
      dynamicHTML =   HTML_BEGIN \
         PROTOCOL_MISMATCH \
         HTML_END;
   }

   // Write out the dynamic VXML script to be processed by OpenVXI
   if (out)
   {
      out->remove(0);
      out->append(dynamicHTML.data());
   }

   return OS_SUCCESS;
}
