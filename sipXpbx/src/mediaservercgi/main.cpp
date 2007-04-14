//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <signal.h>


// APPLICATION INCLUDES
#include "sipdb/SIPDBManager.h"
#include "os/OsDefs.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "os/OsDateTime.h"
#include "os/OsDatagramSocket.h"
#include "cgicc/Cgicc.h"
#include "sipxcgi/CgiValues.h"
#include "net/Url.h"
#include "net/HttpMessage.h"
#include "net/HttpBody.h"
#include "net/MimeBodyPart.h"
#include "net/NetBase64Codec.h"
#include "utl/UtlTokenizer.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/HTMLDefs.h"
#include "mailboxmgr/DepositCGI.h"
#include "mailboxmgr/SosCGI.h"
#include "mailboxmgr/RetrieveCGI.h"
#include "mailboxmgr/LoginCGI.h"
#include "mailboxmgr/ChangePinCGI.h"
#include "mailboxmgr/PlayMessagesCGI.h"
#include "mailboxmgr/MoveMessagesCGI.h"
#include "mailboxmgr/UpdateMessageStatesCGI.h"
#include "mailboxmgr/AutoAttendantCGI.h"
#include "mailboxmgr/SpecialAAMenuCGI.h"
#include "mailboxmgr/DialByNameCGI.h"
#include "mailboxmgr/DeleteMailboxCGI.h"
#include "mailboxmgr/RecycleDeletedMsgsCGI.h"
#include "mailboxmgr/SetActiveGreetingCGI.h"
#include "mailboxmgr/GetAllGreetingsCGI.h"
#include "mailboxmgr/DeleteGreetingCGI.h"
#include "mailboxmgr/SaveGreetingCGI.h"
#include "mailboxmgr/SaveMessage.h"
#include "mailboxmgr/SendByDistListCGI.h"
#include "mailboxmgr/ForwardMessagesCGI.h"
#include "mailboxmgr/ForwardByDistListCGI.h"
#include "mailboxmgr/StatusServerCGI.h"
#include "mailboxmgr/GetNavWebCGI.h"
#include "mailboxmgr/ManageFoldersWebCGI.h"
#include "mailboxmgr/ManageDistributionsWebCGI.h"
#include "mailboxmgr/EditMessageWebCGI.h"
#include "mailboxmgr/TransferToExtnCGI.h"
#include "mailboxmgr/ManageNotificationsWebCGI.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/SaveSystemPromptsCGI.h"
#include "mailboxmgr/SetActiveSystemPromptCGI.h"
#include "mailboxmgr/GetAllSystemPromptsCGI.h"
#include "mailboxmgr/NotificationHelper.h"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MEDIASERVER_LOG     SIPX_LOGDIR "/mediaserver_cgi.log"
#define LOG_FACILITY        FAC_MEDIASERVER_CGI

#if defined(_WIN32)
#define SIGPIPE 13
#endif
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
typedef void (*sighandler_t)(int);

// FUNCTIONS
extern "C" {
    void  sigHandler( int sig_num );
    sighandler_t pt_signal( int sig_num, sighandler_t handler );
}
// GLOBAL VARIABLE INITIALIZATIONS
OsMutex*     gpLockMutex = new OsMutex(OsMutex::Q_FIFO);
UtlBoolean   gClosingIMDB = FALSE;
static UtlBoolean   gInSigHandler = FALSE;
cgicc::Cgicc *gCgi = NULL;
CgiValues *gValues = NULL;

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 * this procedure is slightly different for CGI's where there is
 * no server task
 */
void
closeIMDBConnectionsFromCGI ()
{
    // Critical Section here
    OsLock lock( *gpLockMutex );

    if (!gClosingIMDB)
    {
        // prevent deadlock recursion from multiple
        // signals closing the IMDB
        gClosingIMDB = TRUE;
        // Ensure that this process calls close on the IMDB
        // this will only access the FastDB if it was opened
        // by reference and tables were registers (It checks for
        // pFastDB in its destructor and pFastDB is only created
        // or opened if a user requests a table
        delete SIPDBManager::getInstance();

        delete MailboxManager::getInstance();
    }

}

/**
 * Description:
 * This is a replacement for signal() which registers a signal handler but sets
 * a flag causing system calls ( namely read() or getchar() ) not to bail out
 * upon recepit of that signal. We need this behavior, so we must call
 * sigaction() manually.
 */
sighandler_t
pt_signal( int sig_num, sighandler_t handler)
{
#if defined(__pingtel_on_posix__)
    struct sigaction action[2];
    action[0].sa_handler = handler;
    sigemptyset(&action[0].sa_mask);
    action[0].sa_flags = 0;
    sigaction ( sig_num, &action[0], &action[1] );
    return action[1].sa_handler;
#else
    return signal( sig_num, handler );
#endif
}

/**
 * Description:
 * This is the signal handler, When called this sets the
 * global gShutdownFlag allowing the main processing
 * loop to exit cleanly.
 */
void
sigHandler( int sig_num )
{
    if (!gInSigHandler)
    {
        // Minimize the chance that we loose log data
        OsSysLog::flush();
    }

    if (sig_num == SIGPIPE)
    {
        // ignore this one
        pt_signal( sig_num, SIG_IGN );
        if (!gInSigHandler)
        {
                gInSigHandler = TRUE;
                OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
                OsSysLog::flush();
        }
    }
    else
    {
        // Unregister interest in the signal to prevent recursive callbacks
        pt_signal( sig_num, SIG_DFL );

        if (!gInSigHandler)
        {
                gInSigHandler = TRUE;
                if (SIGTERM == sig_num)
                {
                   OsSysLog::add( LOG_FACILITY, PRI_INFO, "sigHandler: terminate signal received.");
                }
                else
                {
                   OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
                }
                OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: closing IMDB connections" );
                OsSysLog::flush();
        }

        if (!gClosingIMDB)
        {
                closeIMDBConnectionsFromCGI();
                gClosingIMDB = TRUE;
        }
    }
}

OsStatus
getLogFilePath ( UtlString& logFilePath )
{
    OsStatus result = OS_SUCCESS;

    logFilePath = MEDIASERVER_LOG ;

    return result;
}

OsStatus
getMessageIdsFromQuery( UtlString& returnString )
{
   // Get the vector of form entry name/value pairs.
   const std::vector<cgicc::FormEntry> values = **gCgi;
   // Iterate through the vector.
   for (cgicc::const_form_iterator i = values.begin();
        i != values.end();
        i++)
   {
      // Find the pairs whose name starts with a digit.
      if (isdigit((i->getName())[0]))
      {
         // Add the message number to the return string.
         if(!returnString.isNull())
         {
            // Separate numbers in returnString with spaces.
            returnString += " ";
         }
         returnString += i->getName().data();
      }
   }

   OsSysLog::add(LOG_FACILITY, PRI_DEBUG,
                 "getMessageIdsFromQuery: returnString = '%s'",
                 returnString.data());
   return OS_SUCCESS;
}

void
setLogLevel()
{
    UtlString logLevel ;
    MailboxManager * pMailboxManager = MailboxManager::getInstance();
    pMailboxManager->getCustomParameter( PARAM_LOG_LEVEL, logLevel ) ;
    OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "setLogLevel logLevel = '%s'",
                   logLevel.data());
    if( !logLevel.isNull() )
    {
        // Strip the leading and trailing spaces.
        logLevel = logLevel.strip(UtlString::both, ' ') ;

        // Convert to caps.
        logLevel.toUpper();

        OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "setLogLevel: after cleaning, logLevel = '%s'",
                       logLevel.data());
        if( logLevel == LOG_LEVEL_DEBUG )
        {
           OsSysLog::setLoggingPriority(PRI_DEBUG);
        }
        else if( logLevel == LOG_LEVEL_INFO )
        {
           OsSysLog::setLoggingPriority(PRI_INFO);
        }
        else if( logLevel == LOG_LEVEL_NOTICE )
        {
           OsSysLog::setLoggingPriority(PRI_NOTICE);
        }
        else if( logLevel == LOG_LEVEL_WARNING )
        {
           OsSysLog::setLoggingPriority(PRI_WARNING);
        }
        else if( logLevel == LOG_LEVEL_ERROR )
        {
           OsSysLog::setLoggingPriority(PRI_ERR);
        }
        else if( logLevel == LOG_LEVEL_CRITICAL )
        {
           OsSysLog::setLoggingPriority(PRI_CRIT);
        }
        else if( logLevel == LOG_LEVEL_ALERT )
        {
           OsSysLog::setLoggingPriority(PRI_ALERT);
        }
        else if( logLevel == LOG_LEVEL_EMERGENCY )
        {
           OsSysLog::setLoggingPriority(PRI_EMERG);
        }
        else
        {
            OsSysLog::setLoggingPriority(PRI_ERR);
            OsSysLog::add(LOG_FACILITY, PRI_ERR, "Log level set in voicemail.xml.in is not valid. Defaulting to ERR." );
        }
    }
    else
    {
        OsSysLog::setLoggingPriority(PRI_ERR);
        OsSysLog::add(LOG_FACILITY, PRI_ERR, "Failed to read the log level from voicemail.xml.in file. Defaulting to ERR." );
    }
}

int
main(int argc, char* argv[])
{
    gInSigHandler = FALSE;

    // Register Signal handlers to close IMDB
    pt_signal(SIGINT,   sigHandler);    // Trap Ctrl-C on NT
    pt_signal(SIGILL,   sigHandler);
    pt_signal(SIGABRT,  sigHandler);    // Abort signal 6
    pt_signal(SIGFPE,   sigHandler);    // Floading Point Exception
    pt_signal(SIGSEGV,  sigHandler);    // Address access violations signal 11
    pt_signal(SIGTERM,  sigHandler);    // Trap kill -15 on UNIX
#if defined(__pingtel_on_posix__)
    pt_signal(SIGHUP,   sigHandler);    // Hangup
    pt_signal(SIGQUIT,  sigHandler);
    pt_signal(SIGPIPE,  SIG_IGN);    // Handle TCP Failure
    pt_signal(SIGBUS,   sigHandler);
    pt_signal(SIGSYS,   sigHandler);
    pt_signal(SIGXCPU,  sigHandler);
    pt_signal(SIGXFSZ,  sigHandler);
    pt_signal(SIGUSR1,  sigHandler);
    pt_signal(SIGUSR2,  sigHandler);
#endif

 // set directory for log files
    UtlString logFilePath;
    int result = 0;
    if ( getLogFilePath ( logFilePath ) == OS_SUCCESS )
    {
        // Initialize the logger.
        OsSysLog::initialize(0, "mediaservercgi" );
        OsSysLog::setOutputFile(0, logFilePath );
        setLogLevel();

        OsSysLog::add(LOG_FACILITY, PRI_DEBUG,
                      "Mediaserver CGI - Entering main");

        {
           // Log the program name and arguments.
           UtlString argLog("Called as: ");
           int i;
           for (i = 0; i < argc; i++)
           {
             argLog.append("'");
             argLog.append(argv[i]);
             argLog.append("' ");
           }
           OsSysLog::add(LOG_FACILITY, PRI_DEBUG, argLog.data());
        }

        {
           // Log the environment.
           UtlString envLog("Environment: \n");
           int i;
           extern char **environ;
           for (i = 0; environ[i] != NULL; i++)
           {
              envLog.append(environ[i]);
              envLog.append("\n");
              // Print a special message showing QUERY_STRING, which contains
              // the parameters the CGI is called with, as they were sent by
              // the caller.
              if (strncmp(environ[i], "QUERY_STRING=",
                          sizeof ("QUERY_STRING=") - 1) == 0)
              {
                 OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "query = '%s'",
                               environ[i] + sizeof ("QUERY_STRING=") - 1);
              }
           }
           // Don't forget that envLog may contain %'s!
           OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "%s", envLog.data());
        }

        gCgi = new cgicc::Cgicc();
        gValues = new CgiValues(gCgi);
        OsSysLog::add(LOG_FACILITY, PRI_DEBUG,
            "Mediaserver CGI - About to process action");
        CGICommand* cmd = NULL;

        // Determine the source of the request.
        // It could either be WebUI or OpenVXI.
        UtlBoolean requestIsFromWebUI(FALSE);
        const char* fromWebui = gValues->valueOf ( "fromweb" );

        if( fromWebui )
        {
           requestIsFromWebUI = TRUE;
        }
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "main: requestIsFromWebUI = %d", requestIsFromWebUI);

        const char* sActionCGIVar = gValues->valueOf( "eventtype" );
        if ( sActionCGIVar == NULL )
        {
            sActionCGIVar = gValues->valueOf( "action" );
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG, "main: gValues->valueOf(\"eventtype\") == NULL, using gValues->valueOf(\"action\")");
        }
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG, "sActionCGIVar = '%s'",
                      sActionCGIVar);

        // we must specify an action field
        if ( sActionCGIVar != NULL )
        {
            // some of the webui sets upper case characters (submit forms etc.)
            UtlString action ( sActionCGIVar );
            action.toLower();

            OsSysLog::add(LOG_FACILITY, PRI_INFO, "Mediaserver CGI - action=%s ", sActionCGIVar);

            // the proxy appends the from address so it should always be a URL
            Url fromUrl;
            const char* sFromCGIVar = gValues->valueOf ( "from" );
            if( sFromCGIVar )
            {
                fromUrl = sFromCGIVar;
                OsSysLog::add( LOG_FACILITY, PRI_INFO, "Mediaserver CGI - from='%s'", sFromCGIVar);

            }
            else
            {
                OsSysLog::add( LOG_FACILITY, PRI_INFO, "Mediaserver CGI - from field not set");
            }


            // The mailbox is a URL unless we log in over the web
            // or alternatively the mapping rules do not specify
            // the realm (in which case the default realm will
            // be appended in the MailboxManager
            UtlString mailbox;
	    const char* sMailboxCGIVar = gValues->valueOf ( "mailbox" );
	    if ( sMailboxCGIVar )
            {
		mailbox = sMailboxCGIVar;
		OsSysLog::add( LOG_FACILITY, PRI_INFO, "Mediaserver CGI - mailbox=%s \n", mailbox.data());

                if( requestIsFromWebUI )
                {   // this will just be a userid without the @domain part
   		   setenv( "REMOTE_USER", mailbox.data(), 1);
                   OsSysLog::add( LOG_FACILITY, PRI_INFO, "Mediaserver CGI - mailbox=%s \n", mailbox.data());
                }
 	    }

            // Determine the type of cgi action
            if ( action == "sos" )
            {
                if ( sFromCGIVar )
                {
                    // Create the appropriate Command Object
                    cmd = new SosCGI( fromUrl );
                }
                else
                {
                   OsSysLog::add( LOG_FACILITY, PRI_ERR, "Mediaserver CGI - from field is not set for emergency service");
                }
            }
            else if ( action == "deposit" )
            {
                // Deposit requires from and mailbox parameters
                if ( sFromCGIVar && !mailbox.isNull() )
                {
                    // Create the appropriate Command Object
                    cmd = new DepositCGI ( fromUrl, mailbox );
                }
            } else if ( action == "retrieve" )
            {
               const char* sRetrieveMailbox =
                  requestIsFromWebUI ? gValues->valueOf( "REMOTE_USER" ) :
                  // If mailbox was passed in the URL, use it
                  !mailbox.isNull() ? mailbox.data() :
                  sFromCGIVar;
               OsSysLog::add(LOG_FACILITY, PRI_INFO,
                             "Mediaserver CGI - For Retrieve, using mailbox '%s'",
                             sRetrieveMailbox);

               if ( sRetrieveMailbox )
               {
                  UtlString retrieveMailbox( sRetrieveMailbox );

                  cmd = new RetrieveCGI( requestIsFromWebUI, retrieveMailbox );
               }
            } else if ( action == "autoattendant" )
            {
                if ( sFromCGIVar )
                {
                    UtlString strName;
                    const char* sDigits = gValues->valueOf ( "digits" );

                    const char* sName = gValues->valueOf ( "name" );
                    strName = sName;
                    OsSysLog::add( LOG_FACILITY, PRI_INFO, "Mediaserver CGI - Autoattendant app name = '%s'", strName.data());

                    // Create the appropriate Command Object
                    cmd = new AutoAttendantCGI( fromUrl, strName, sDigits );
                }
            } else if ( action == "dialbyname" )
            {
                const char* sDigitsCGIVar = gValues->valueOf ( "digits" );
                const char* sFromDepositCGIVar = gValues->valueOf ( "fromdeposit" );
                UtlBoolean isFromDeposit = FALSE ;
                if( sFromDepositCGIVar )
                {
                    UtlString strFromDeposit ( sFromDepositCGIVar ) ;
                    if( strFromDeposit.compareTo("yes", UtlString::ignoreCase) == 0 )
                        isFromDeposit = TRUE ;
                }

                if ( sFromCGIVar && sDigitsCGIVar )
                {
                    UtlString digits(sDigitsCGIVar);
                    // Create the appropriate Command Object
                    cmd = new DialByNameCGI( fromUrl, digits, isFromDeposit );
                }
            } else if ( action == "setspecialmenu" )
            {
                const char* sOption = gValues->valueOf ( "setoption" );
                OsSysLog::add( LOG_FACILITY, PRI_INFO, "Mediaserver CGI - Set special AA menu to '%s'", sOption);

                // Create the appropriate Command Object
                cmd = new SpecialAAMenuCGI( sOption );
            } else if ( action == "login" )
            {
                UtlString extension, pin;
                if( !requestIsFromWebUI )
                {
                    const char* sextension = gValues->valueOf ( "extension" );
                    const char* spin = gValues->valueOf ( "pin" );

                    if (sextension && spin)
                    {
                        extension = sextension ;
                        pin = spin;
                    }
                }
                else
                {
                    extension = gValues->valueOf("REMOTE_USER") ;
                }

                // Create the appropriate Command Object
                cmd = new LoginCGI ( requestIsFromWebUI, extension, pin );

            } else if ( action == "changepin" )
            {
               const char* suserid = gValues->valueOf ( "extension" );
               const char* spassword = gValues->valueOf ( "pin" );
               const char* snewpassword = gValues->valueOf ( "newpin" );

               // Create the appropriate Command Object
               cmd = new ChangePinCGI ( suserid, spassword, snewpassword );

            } else if ( action == "playmsg" )
            {
                const char* sCategoryCGIVar = gValues->valueOf ( "category" );
                const char* sBlocksizeCGIVar = gValues->valueOf ( "blocksize" );
                const char* sNextblockhandleCGIVar = gValues->valueOf ( "nextblockhandle" );
                const char* sStatusCGIVar = gValues->valueOf ( "status" );
                const char* sUnheardMsgIdList = gValues->valueOf( "unheardMsgIdList" );
                const char* sFrameSet = gValues->valueOf( "frameset" );

                if( !sFrameSet )
                    sFrameSet = "no" ;
                if( !sStatusCGIVar )
                    sStatusCGIVar = "-1" ;
                if( requestIsFromWebUI && !sBlocksizeCGIVar )
                    sBlocksizeCGIVar = "-1" ;
                if( !sUnheardMsgIdList )
                    sUnheardMsgIdList = "0" ;

                if ( sFromCGIVar && !mailbox.isNull() && sCategoryCGIVar &&
                     sBlocksizeCGIVar && sNextblockhandleCGIVar )
                {
                    // Create the appropriate Command Object
                    cmd = new PlayMessagesCGI(
                        requestIsFromWebUI,
                        mailbox,
                        fromUrl,
                        sCategoryCGIVar,
                        sBlocksizeCGIVar,
                        sNextblockhandleCGIVar,
                        sStatusCGIVar,
                        sUnheardMsgIdList,
                        sFrameSet);
                }
                else
                {
                   OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "main: could not call new PlayMessagesCGI due to missing parameters");
                }
            } else if ( ( action == "movemsg" ) || ( action.compareTo("move", UtlString::ignoreCase) == 0) )
            {
                const char* sFromFolderCGIVar = gValues->valueOf ( "fromfolder" );
                const char* sToFolderCGIVar = gValues->valueOf( "tofolder" );
                const char* sMaintainstatusCGIVar = gValues->valueOf( "maintainstatus" );
                const char* sNextBlockHandle = gValues->valueOf( "nextblockhandle" );

                // Parameter 'nextblockhandle' is defined only for the WebUI
                if( !sNextBlockHandle )
                    sNextBlockHandle = "-1" ;

                // Retrieve the messages the user selected
                // message ids are sent as request parameters.
                UtlString messageIds;
                if( requestIsFromWebUI )
                {
                   getMessageIdsFromQuery( messageIds ) ;
                }
                else
                {
                   messageIds = gValues->valueOf( "messageids" );
                }

                if ( sFromFolderCGIVar && sToFolderCGIVar &&
                     !mailbox.isNull() && !messageIds.isNull() &&
                     sMaintainstatusCGIVar)
                {
                    // Create the appropriate Command Object
                    cmd = new MoveMessagesCGI(
                                    requestIsFromWebUI,
                                    mailbox,
                                    sFromFolderCGIVar,
                                    sToFolderCGIVar,
                                    messageIds,
                                    sMaintainstatusCGIVar,
                                    sNextBlockHandle);
                }
            } else if ( action == "updatestatus" )
            {
                const char* sCategoryCGIVar = gValues->valueOf ( "category" );
                const char* sMessageIdsCGIVar = gValues->valueOf( "messageidlist" );
                const char* sLinkInEmail = gValues->valueOf( "emaillink" );

                UtlBoolean bLinkInEmail(FALSE);
                if( sLinkInEmail )
                {
                    UtlString linkInEmail ( sLinkInEmail );
                    if( linkInEmail.compareTo("yes", UtlString::ignoreCase) == 0 )
                        bLinkInEmail = TRUE ;
                }

                if (sCategoryCGIVar && !mailbox.isNull() && sMessageIdsCGIVar)
                {
                    UtlString messageIds(sMessageIdsCGIVar);
                    UtlString category(sCategoryCGIVar);

                    // Create the appropriate Command Object
                    cmd = new UpdateMessageStatesCGI(
                        requestIsFromWebUI,
                        bLinkInEmail,
                        mailbox,
                        category,
                        messageIds );
                }
            } else if ( action == "recycledeleted" || (action.index("empty",0,UtlString::ignoreCase)!= UTL_NOT_FOUND))
            {
                const char* sNextBlockHandle = gValues->valueOf( "nextblockhandle" );
                const char* sMessageIdsCGIVar = gValues->valueOf( "messageids" );

                // Parameters 'nextblockhandle' is defined only for the WebUI
                if( !sNextBlockHandle )
                    sNextBlockHandle = "-1" ;

                if( !sMessageIdsCGIVar )
                    sMessageIdsCGIVar = "-1" ;

                if ( !mailbox.isNull())
                {
                    // Create the appropriate Command Object
                    cmd = new RecycleDeletedMsgsCGI( requestIsFromWebUI,
                                                     mailbox,
                                                     sMessageIdsCGIVar,
                                                     sNextBlockHandle );
                }
            } else if ( action.compareTo("delete", UtlString::ignoreCase) == 0 )
            {
                // 'Delete' is called from the Web UI.

                // Retrieve the messages the user selected.
                UtlString messageIds;
                getMessageIdsFromQuery( messageIds ) ;
                const char* sMessageIdsCGIVar = messageIds.data() ;

                // 'deletemode' suggests the kind of delete operation to be performed:

                // if deletemode = purge, then the request was made from the deleted folder page.
                // User wants to permanently delete (hence 'purge') the selected messages
                // from the deleted folder.

                // if deletemode = delete, then the request was made from any folder page
                // other than the deleted folder. User wants to move the messages to
                // the deleted folder.

                const char* sDeleteModeCGIVar = gValues->valueOf( "deletemode" );

                UtlString strDeleteMode ;
                if( sDeleteModeCGIVar )
                    strDeleteMode = sDeleteModeCGIVar ;
                else
                    strDeleteMode = "delete" ;

                const char* sNextBlockHandle = gValues->valueOf( "nextblockhandle" );

                // Parameter 'nextblockhandle' is defined only for the WebUI
                if( !sNextBlockHandle )
                    sNextBlockHandle = "-1" ;

                if( strDeleteMode.compareTo("purge", UtlString::ignoreCase) == 0 )
                {
                    // Permanently delete messages from the deleted folder
                    if( !mailbox.isNull() && sMessageIdsCGIVar )
                    {
                        cmd = new RecycleDeletedMsgsCGI( requestIsFromWebUI,
                                                         mailbox,
                                                         sMessageIdsCGIVar,
                                                         sNextBlockHandle);
                    }
                }
                else
                {
                    // Move messages to the deleted folder.
                    const char* sFromFolderCGIVar = gValues->valueOf ( "fromfolder" );
                    const char* sMaintainstatusCGIVar = gValues->valueOf( "maintainstatus" );
                    const char* sToFolderCGIVar = "deleted" ;

                    if ( sFromFolderCGIVar && sMaintainstatusCGIVar &&
                         !mailbox.isNull() && sMessageIdsCGIVar )
                    {
                        // Create the appropriate Command Object
                        cmd = new MoveMessagesCGI(
                                        requestIsFromWebUI,
                                        mailbox,
                                        sFromFolderCGIVar,
                                        sToFolderCGIVar,
                                        sMessageIdsCGIVar,
                                        sMaintainstatusCGIVar,
                                        sNextBlockHandle
                                        );
                    }
                }

            } else if ( action == "setactivegreeting" )
            {
                const char* sGreetingTypeCGIVar = gValues->valueOf( "greetingtype" );

                if (sGreetingTypeCGIVar && !mailbox.isNull())
                {
                    // Create the appropriate Command Object
                    cmd = new SetActiveGreetingCGI( requestIsFromWebUI,
                                                    mailbox,
                                                    sGreetingTypeCGIVar  );
                }
            } else if ( action == "getallgreetings" )
            {
                UtlString status = "-1";

                const char *s_status = gValues->valueOf( "status" );
                if (!mailbox.isNull())
                {
                    if( s_status )
                        status = s_status ;

                    // Create the appropriate Command Object
                    cmd = new GetAllGreetingsCGI(
                        requestIsFromWebUI,
                        mailbox,
                        status );
                }
            } else if ( action == "deletegreeting" )
            {
                const char* greetingType = gValues->valueOf( "greetingtype" );
                const char* isActive = gValues->valueOf( "active" );

                UtlBoolean isActiveGreeting = FALSE ;
                if( isActive)
                    isActiveGreeting = TRUE ;

                if (greetingType && !mailbox.isNull())
                {
                    // Create the appropriate Command Object
                    cmd = new DeleteGreetingCGI(
                        requestIsFromWebUI,
                        mailbox,
                        greetingType,
                        isActiveGreeting );
                }
            } else if ( action == "savegreeting" )
            {
                const char* greetingType = gValues->valueOf ( "greetingtype" );
                const char* size = gValues->valueOf ( "size" );
                const char* encodedWavStr = gValues->valueOf ( "data" );
                if (size && encodedWavStr && greetingType)
                {
                    int decodedLength = atoi(size);
                    char* decodedWavBytes = new char[decodedLength + 1];
                    memcpy(decodedWavBytes, encodedWavStr, decodedLength);
                    cmd = new SaveGreetingCGI(
                                mailbox,
                                greetingType,
                                decodedWavBytes,
                                decodedLength );
                    delete[] decodedWavBytes;
                }
            } else if ( action == "savemessage" )
            {
                if ( sFromCGIVar)
                {
                    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "savemessage param: from=%s ", sFromCGIVar );
                }
                const char* duration = gValues->valueOf ( "duration" );
                const char* size = gValues->valueOf ( "size" );
                const char* timestamp = gValues->valueOf ( "timestamp" );
                const char* encodedWavStr = gValues->valueOf ( "vm" );
                const char* termChar = gValues->valueOf ( "termchar" );
                  if (duration)
                {
                    OsSysLog::add(LOG_FACILITY, PRI_INFO, " duration=%sms", duration );
                }
                if (size && encodedWavStr && timestamp)
                {
                    int decodedLength = atoi(size);
                    char* decodedWavBytes = new char[decodedLength + 1];
                    memcpy(decodedWavBytes, encodedWavStr, decodedLength);
                    cmd = new SaveMessage(
                        fromUrl, mailbox,
                        duration,
                        timestamp,
                        termChar,
                        decodedWavBytes,
                        decodedLength);
                    delete[] decodedWavBytes;
                }
            } else if ( action == "sendbydistlist" )
            {
                const char* frommailbox = gValues->valueOf ( "frommailbox" );
                const char* distList = gValues->valueOf ( "distlist" );
                const char* duration = gValues->valueOf ( "duration" );
                const char* size = gValues->valueOf ( "size" );
                const char* timestamp = gValues->valueOf ( "timestamp" );
                const char* encodedWavStr = gValues->valueOf ( "vm" );
                const char* termChar = gValues->valueOf ( "termchar" );

                OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "sendbydistlist param: frommailbox = %s ", frommailbox );
                OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "sendbydistlist param: distribution list = %s ", distList );

                if (duration)
                {
                    OsSysLog::add(LOG_FACILITY, PRI_INFO, " duration=%sms", duration );
                }
                if (size && encodedWavStr && timestamp)
                {
                    int decodedLength = atoi(size);
                    char* decodedWavBytes = new char[decodedLength + 1];
                    memcpy(decodedWavBytes, encodedWavStr, decodedLength);
                    cmd = new SendByDistListCGI(
                        fromUrl,
                        frommailbox,
                        distList,
                        duration,
                        timestamp,
                        termChar,
                        decodedWavBytes,
                        decodedLength);
                    delete[] decodedWavBytes;
                }
            } else if ( action == "forward" )
            {
                const char* encodedWavStr = gValues->valueOf ( "comments" );
                const char* duration = gValues->valueOf ( "duration" );
                const char* timestamp = gValues->valueOf ( "timestamp" );
                const char* size = gValues->valueOf ( "size" );
                const char* fromMailbox = gValues->valueOf ( "frommailbox" );
                const char* fromFolder = gValues->valueOf ( "fromfolder" );
                const char* messageIds = gValues->valueOf ( "messageids" );
                const char* toExtension = gValues->valueOf ( "toextension" );

                if( !size)
                {
                    UtlString   strTimeStamp ;
                    OsDateTime::getLocalTimeString(strTimeStamp) ;
                    // user has not recorded any comments
                    cmd = new ForwardMessagesCGI(
                                "0",
                                "0",
                                strTimeStamp,
                                0,
                                fromMailbox,
                                fromFolder,
                                messageIds,
                                toExtension );
                }
                else if (size && encodedWavStr && fromMailbox && toExtension && messageIds)
                {
                    int decodedLength = atoi(size);
                    char* decodedWavBytes = new char[decodedLength + 1];
                    memcpy(decodedWavBytes, encodedWavStr, decodedLength);

                    cmd = new ForwardMessagesCGI(
                                decodedWavBytes,
                                duration,
                                timestamp,
                                decodedLength,
                                fromMailbox,
                                fromFolder,
                                messageIds,
                                toExtension );
                    delete[] decodedWavBytes;
                }
            } else if ( action == "forwardbydistlist" )
            {
                const char* encodedWavStr = gValues->valueOf ( "comments" );
                const char* duration = gValues->valueOf ( "duration" );
                const char* timestamp = gValues->valueOf ( "timestamp" );
                const char* size = gValues->valueOf ( "size" );
                const char* fromMailbox = gValues->valueOf ( "frommailbox" );
                const char* fromFolder = gValues->valueOf ( "fromfolder" );
                const char* messageIds = gValues->valueOf ( "messageids" );
                const char* toDistList = gValues->valueOf ( "selectdistlist" );

                if( !size) 
                {
                    UtlString   strTimeStamp ;
                    OsDateTime::getLocalTimeString(strTimeStamp) ;
                    // user has not recorded any comments
                    cmd = new ForwardByDistListCGI(
                                "0",
                                "0",
                                strTimeStamp,
                                0, 
                                fromMailbox,
                                fromFolder,
                                messageIds,
                                toDistList );
                }
                else if (size && encodedWavStr && fromMailbox && toDistList && messageIds)
                {
                    int decodedLength = atoi(size);
                    char* decodedWavBytes = new char[decodedLength + 1];
                    memcpy(decodedWavBytes, encodedWavStr, decodedLength);

                    cmd = new ForwardByDistListCGI(
                                decodedWavBytes,
                                duration,
                                timestamp,
                                decodedLength,
                                fromMailbox,
                                fromFolder,
                                messageIds,
                                toDistList );
                    delete[] decodedWavBytes;
                }
            } else if ( action == "deletemailbox" )
            {
                if ( !mailbox.isNull() )
                {
                    cmd = new DeleteMailboxCGI( mailbox );
                }
            } else if ( action == "message-summary" )
            {
                // Request message waiting indicator status
                const char* sIdentity = gValues->valueOf( "identity" );

                if ( sIdentity != NULL )
                {
                    mailbox = sIdentity;
                    cmd = new StatusServerCGI ( mailbox );
                }
            } else if ( action == "getnav" )
            {
                if ( !mailbox.isNull() )
                {
                    // Create the appropriate Command Object
                    cmd = new GetNavWebCGI ( mailbox );
                }
            } else if ( action == "getpersonalfolders" ||
                        action == "getfolders" ||
                        action == "addfolder" ||
                        action == "editfolder" ||
                        action == "deletefolder" ||
                        action == "getaddfolderui" ||
                        action == "geteditfolderui" )
            {
                UtlString status = "-1" ;
                UtlString strNewFolderName = "-1" ;
                UtlString strOldFolderName = "-1" ;

                const char *s_status = gValues->valueOf( "status" );
                const char *newfoldername = gValues->valueOf( "newfoldername" );
                const char *oldfoldername = gValues->valueOf ( "oldfoldername" );

                // These are optional parameters
                if( newfoldername )
                    strNewFolderName = newfoldername ;

                if( oldfoldername )
                    strOldFolderName = oldfoldername ;

                if( s_status )
                    status = s_status ;

                if( !mailbox.isNull() )
                {

                    // Create the appropriate Command Object
                    cmd = new ManageFoldersWebCGI ( mailbox,
                                                    action,
                                                    status,
                                                    strNewFolderName,
                                                    strOldFolderName
                                                   );
                }
            } else if ( action == "editmsg" )
            {
                const char* foldername = gValues->valueOf( "foldername" );
                const char* messageid = gValues->valueOf( "messageid" );
                const char* subject = gValues->valueOf( "subject" );
                const char* submit = gValues->valueOf( "formsubmitted" );
                const char* nextblockhandle = gValues->valueOf( "nextblockhandle" );

                UtlBoolean formsubmitted = FALSE ;
                if( submit )
                {
                    UtlString strSubmit = submit ;
                    if( strSubmit.compareTo("yes", UtlString::ignoreCase) == 0 )
                        formsubmitted = TRUE ;
                }

                if ( !mailbox.isNull() && foldername && messageid && subject && nextblockhandle )
                {
                    // Create the appropriate Command Object
                    cmd = new EditMessageWebCGI (   mailbox,
                                                    foldername,
                                                    messageid,
                                                    subject,
                                                    nextblockhandle,
                                                    formsubmitted);
                }
            } else if ( action == "transfer" )
            {
                const char* extensionCGIVar = gValues->valueOf( "extension" );

                if ( extensionCGIVar )
                {
                    UtlString extension( extensionCGIVar ) ;
                    // Create the appropriate Command Object
                    cmd = new TransferToExtnCGI ( extension );
                }
            } else if ( action == "managenotifications" ||
                        action == "geteditnotificationui" ||
                        action == "getaddnotificationui" ||
                        action == "addnotification" ||
                        action == "editnotification" ||
                        action == "deletenotification"
                        )
            {
                const char* status = gValues->valueOf( "status" ) ;
                const char* contactAddress = gValues->valueOf( "contact" ) ;
                //const char* contactType = gValues->valueOf( "type" ) ;
                const char* sendAttachments = gValues->valueOf( "attachments" ) ;
                const char* newContactAddress = gValues->valueOf( "newcontact" ) ;
                //const char* newContactType = gValues->valueOf( "newtype" ) ;

                if( !status )
                    status = "" ;

                if( !contactAddress )
                    contactAddress = "-1" ;

                //if( !contactType )
                //    contactType = "email" ;

                if( !newContactAddress )
                    newContactAddress = "-1" ;

                //if( !newContactType )
                //    newContactType = "email" ;

                if( !sendAttachments )
                        sendAttachments = "no" ;

                if ( !mailbox.isNull())
                {
                    // Create the appropriate Command Object
                    cmd = new ManageNotificationsWebCGI(    mailbox,
                                                            action,
                                                            status,
                                                            contactAddress,
                                                            "email",
                                                            newContactAddress,
                                                            "email",
                                                            sendAttachments
                                                       );
                }
            } else if ( action == "managedistributions" ||
                        action == "geteditdistributionui" ||
                        action == "getadddistributionui" ||
                        action == "adddistribution" ||
                        action == "editdistribution" ||
                        action == "deletedistribution" )
            {
                const char* status = gValues->valueOf( "status" ) ;
                const char* index = gValues->valueOf( "index" ) ;
                const char* distribution = gValues->valueOf( "distribution" ) ;

                if( !status )
                    status = "" ;

                if ( !mailbox.isNull())
                {
                    // Create the appropriate Command Object
                    cmd = new ManageDistributionsWebCGI( mailbox,
                                                         action,
                                                         status,
                                                         index,
                                                         distribution );
                }
            } else if ( action == "savesystemprompt" )
            {
                const char* promptType = gValues->valueOf ( "prompttype" );
                const char* size = gValues->valueOf ( "size" );
                const char* encodedWavStr = gValues->valueOf ( "data" );
                if (size && encodedWavStr && promptType)
                {
                    int decodedLength = atoi(size);
                    char* decodedWavBytes = new char[decodedLength + 1];
                    memcpy(decodedWavBytes, encodedWavStr, decodedLength);
                    cmd = new SaveSystemPromptsCGI(
                                promptType,
                                decodedWavBytes,
                                decodedLength );
                    delete[] decodedWavBytes;
                }
            } else if ( action == "setsystemprompt" )
            {
                const char* sGreetingTypeCGIVar = gValues->valueOf( "prompttype" );

                if (sGreetingTypeCGIVar )
                {
                    // Create the appropriate Command Object
                    cmd = new SetActiveSystemPromptCGI( requestIsFromWebUI,
                                                        sGreetingTypeCGIVar  );
                }
            } else if ( action == "getallsystemprompts" )
            {
                const char* sGreetingTypeCGIVar = gValues->valueOf( "prompttype" );

                if (sGreetingTypeCGIVar )
                {
                    // Create the appropriate Command Object
                    cmd = new GetAllSystemPromptsCGI(    requestIsFromWebUI,
                                                        "-1",
                                                        sGreetingTypeCGIVar  );
                }
            }



            if ( cmd != NULL )
            {
                UtlString outStr;
                if ( cmd->execute(&outStr) == OS_SUCCESS )
                {
                    OsSysLog::add(
                        LOG_FACILITY, PRI_INFO,
                        "CGI executed successfully.");
                    if (!outStr.isNull())
                    {
                        printf( "%s", outStr.data() );
                        OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "main: outStr = '%s'",
                                       outStr.data() );
                    }
                } else if( requestIsFromWebUI )
                {
                    UtlString redirectUrl ;
                    MailboxManager * pMailboxManager = MailboxManager::getInstance();
                    pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;
                    redirectUrl +=  VOICEMAIL_NOT_ENABLED_URL ;

                    printf( HTML_BEGIN );
                    printf( REDIRECT_SCRIPT_BEGIN );
                    printf( redirectUrl.data() );
                    printf( REDIRECT_SCRIPT_END );
                    printf( HTML_END );
                    OsSysLog::add( LOG_FACILITY, PRI_ERR, "main: ERR in executing the CGI: action = '%s', cmd->execute() returned error, requestIsFromWebUI = 0",
                                   action.data());
                    result = -1;
                } else
                {
                    // create a file upload log containg the entire
                    // request multipart mime encoded contents
                    // logHttpRequestData ("/requestbad.log" );
                    UtlString vxmlBody = (UtlString)VXML_BODY_BEGIN + VXML_FAILURE_SNIPPET + VXML_END;
                    UtlString responseHeaders;
                    MailboxManager::getResponseHeaders(vxmlBody.length(), responseHeaders);
                    printf ( responseHeaders.data() );
                    printf ( vxmlBody.data() );
                    OsSysLog::add( LOG_FACILITY, PRI_ERR, "main: ERR in executing the CGI: action = '%s', cmd->execute() returned error, requestIsFromWebUI = 1",
                                   action.data());
                    result = -1;
                }

                delete cmd;
            } else if( requestIsFromWebUI )
            {
                UtlString redirectUrl ;
                MailboxManager* pMailboxManager = MailboxManager::getInstance();
                pMailboxManager->getMediaserverURLForWeb( redirectUrl ) ;
                redirectUrl +=  VOICEMAIL_NOT_ENABLED_URL ;

                printf( HTML_BEGIN );
                printf( REDIRECT_SCRIPT_BEGIN );
                printf( redirectUrl.data() );
                printf( REDIRECT_SCRIPT_END );
                printf( HTML_END );
                OsSysLog::add( LOG_FACILITY, PRI_ERR, "main: ERR in executing the CGI: action = '%s', cmd = NULL, requestIsFromWebUI = 1",
                                   action.data());
                result = -1;
            }
            else
            {
                UtlString vxmlBody = (UtlString)VXML_BODY_BEGIN + VXML_FAILURE_SNIPPET + VXML_END;
                UtlString responseHeaders;
                MailboxManager::getResponseHeaders(vxmlBody.length(), responseHeaders);
                printf ( responseHeaders.data() );
                printf ( vxmlBody.data() );
                OsSysLog::add( LOG_FACILITY, PRI_ERR, "main: ERR in executing the CGI: action = '%s', cmd = NULL, requestIsFromWebUI = 0",
                                   action.data());
                result = -1;
            }
        }
        else
        {
           OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                         "main: CGI request had no action specified: QUERY_STRING = '%s'",
                         getenv("QUERY_STRING"));
        }
    }

    // Cleanly close any connections to the IMDB
    closeIMDBConnectionsFromCGI ();

    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Mediaserver CGI - leaving main");
    OsSysLog::flush();
    return result;
}
