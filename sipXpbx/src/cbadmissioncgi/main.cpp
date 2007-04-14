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
#include "cbadmissioncgi/ConferenceManager.h"
#include "cbadmissioncgi/ConferenceBridgeCGI.h"
#include "cbadmissioncgi/LoginCGI.h"
#include "cbadmissioncgi/TransferCGI.h"


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CBADMISSION_LOG     SIPX_LOGDIR "/cbadmission_cgi.log"
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
static UtlBoolean   gInSigHandler = FALSE;
cgicc::Cgicc *gCgi = NULL;
CgiValues *gValues = NULL;

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
                OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
                OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: closing IMDB connections" );
                OsSysLog::flush();
        }
    }
}

OsStatus
getLogFilePath ( UtlString& logFilePath )
{
    OsStatus result = OS_SUCCESS;

    logFilePath = CBADMISSION_LOG ;

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
    ConferenceManager * pConferenceManager = ConferenceManager::getInstance();
    pConferenceManager->getCustomParameter( PARAM_LOG_LEVEL, logLevel ) ;
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
        OsSysLog::initialize(0, "cbadmissioncgi" );
        OsSysLog::setOutputFile(0, logFilePath );
        setLogLevel();

        OsSysLog::add(LOG_FACILITY, PRI_DEBUG,
                      "CB Admission CGI - Entering main");

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
            "CB Admission CGI - About to process action");
        CGICommand* cmd = NULL;

        const char* sActionCGIVar = gValues->valueOf( "eventtype" );
        if ( sActionCGIVar == NULL )
        {
            sActionCGIVar = gValues->valueOf( "action" );
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG, "main: gValues->valueOf(\"eventtype\") == NULL, using gValues->valueOf(\"action\")");
        }
        
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG, "sActionCGIVar = '%s'", sActionCGIVar);

        // we must specify an action field
        if ( sActionCGIVar != NULL )
        {
            // some of the webui sets upper case characters (submit forms etc.)
            UtlString action ( sActionCGIVar );
            action.toLower();

            OsSysLog::add(LOG_FACILITY, PRI_INFO, "CB Admission CGI - action = %s ", sActionCGIVar);

            // Get the contact field
            //const char* sContactCGIVar = gValues->valueOf ( "contact" );
            const char* sContactCGIVar = gValues->valueOf ( "from" );
            if( sContactCGIVar )
            {
                OsSysLog::add( LOG_FACILITY, PRI_INFO, "CB Admission CGI - contact = %s \n", sContactCGIVar);
            }
            else
            {
                OsSysLog::add( LOG_FACILITY, PRI_INFO, "CB Admission CGI - contact field not set\n");
            }

            // Get the access field
            UtlString access;
            const char* sAccessCGIVar = gValues->valueOf ( "access" );
            if( sAccessCGIVar )
            {
                access = sAccessCGIVar;
                OsSysLog::add( LOG_FACILITY, PRI_INFO, "CB Admission CGI - access = %s \n", sAccessCGIVar);
            }
            else
            {
                OsSysLog::add( LOG_FACILITY, PRI_INFO, "CB Admission CGI - access field not set\n");
            }

            // Determine the type of cgi action
            if ( action == "conferencebridge" )
            {
                if ( sContactCGIVar )
                {
                    UtlString confId = UtlString( gValues->valueOf ( "confid" ) );
                    UtlString appName( gValues->valueOf ( "name" ) );
                    OsSysLog::add( LOG_FACILITY, PRI_INFO, "CB Admission CGI - Conference bridge basic admission control app name = %s \n",
                                   appName.data());

                   // Create the appropriate Command Object
                    cmd = new ConferenceBridgeCGI( appName, sContactCGIVar, confId, access );
                }
            } else if ( action == "login" )
            {
                const char* contact = gValues->valueOf ( "contact" );
                const char* confId = gValues->valueOf ( "confid" );
                const char* accessCode = gValues->valueOf ( "accesscode" );

                // Create the appropriate Command Object
                cmd = new LoginCGI ( contact, confId, accessCode );
            } else if ( action == "transfer" )
            {
                const char* urlCGIVar = gValues->valueOf( "conferenceurl" );

                if ( urlCGIVar )
                {
                    // Create the appropriate Command Object
                    cmd = new TransferCGI ( urlCGIVar );
                }
            }

            if ( cmd != NULL )
            {
                UtlString outStr;
                if ( cmd->execute(&outStr) == OS_SUCCESS )
                {
                    OsSysLog::add(
                        LOG_FACILITY, PRI_INFO,
                        "CB Admission CGI executed successfully.\n");
                    if (!outStr.isNull())
                    {
                        printf( "%s", outStr.data() );
                        OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "main: outStr = '%s'",
                                       outStr.data() );
                    }
                }
                else
                {
                    // create a file upload log containg the entire
                    // request multipart mime encoded contents
                    // logHttpRequestData ("/requestbad.log" );
                    UtlString vxmlBody = (UtlString)VXML_BODY_BEGIN + VXML_FAILURE_SNIPPET + VXML_END;
                    UtlString responseHeaders;
                    ConferenceManager::getResponseHeaders(vxmlBody.length(), responseHeaders);
                    printf ( responseHeaders.data() );
                    printf ( vxmlBody.data() );
                    OsSysLog::add( LOG_FACILITY, PRI_ERR, "main: ERR in executing the CGI: action = '%s', cmd->execute() returned error, requestIsFromWebUI = 1\n",
                                   action.data());
                    result = -1;
                }

                delete cmd;
            }
        }
        else
        {
           OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                         "main: CGI request had no action specified: QUERY_STRING = '%s'",
                         getenv("QUERY_STRING"));
        }
    }

    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "CB Admission CGI - leaving main\n");
    OsSysLog::flush();
    return result;
}
