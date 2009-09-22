//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// System includes
#include "os/OsDefs.h"
#include <getopt.h>
#if defined(_WIN32)
#   include <windows.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#endif

// APPLICATION INCLUDES
#include <net/XmlRpcRequest.h>
#include <os/OsSysLog.h>
#include <os/OsTask.h>

// CONSTANTS
#define HTTP_PORT               8200    // Default HTTP port

int HttpPort = HTTP_PORT;
enum Verbosity
{
   Quiet,
   Normal,
   Verbose
} Feedback = Quiet;

const char* LogFile = "xmlrpcClient.log";
const char* xmlrpcServer = "127.0.0.1";

void initLogger(char* argv[])
{
    OsSysLog::initialize(0, // do not cache any log messages in memory
                         argv[0]); // name for messages from this program
    OsSysLog::setOutputFile(0, // no cache period
                            LogFile); // log file name
    switch (Feedback)
    {
       case Quiet:
          OsSysLog::setLoggingPriority(PRI_WARNING);
          break;
       case Normal:
          OsSysLog::setLoggingPriority(PRI_INFO);
          break;
       case Verbose:
          OsSysLog::setLoggingPriority(PRI_DEBUG);
          break;
    }

   OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
}

void showHelp(char* argv[])
{
   osPrintf("Usage: \n"
          "   %s:\n"
          "   [ {-p|--port} <http-port> ]\n"
          "   [ {-v|--verbose} | {-q|--quiet} ]\n"
          "   [ {-h|-?|--help} ]\n"
          "   [ <xmlrpc server> ]\n"
          , argv[0]
          );
}

void parseArgs(int argc, char* argv[])
{
   int optResult = 0;

   const char* short_options = "p:l:vqh";

   const struct option long_options[] =
      {
         {"verbose", 0, 0, 'v'},
         {"quiet",   0, 0, 'q'},
         {"help",    0, 0, 'h'},
         {"port",     1, 0, 'p'},
         {"log",     1, 0, 'l'},
         {0, 0, 0, 0}
      };

   while ((optResult = getopt_long (argc, argv, short_options, long_options, NULL)
           ) >= 0)
   {
      char* optend;

      switch (optResult)
      {
         case 'p':
            HttpPort = strtoul(optarg, &optend, 10);
            if ( '\0' != *optend )
            {
               fprintf( stderr, "Invalid HTTP port %s\n", optarg );
               exit(1);
            }
            break;

         case 'l':
            LogFile = optarg;
            break;

         case 'v':
            Feedback = Verbose;
            break;

         case 'q':
            Feedback = Quiet;
            break;

         case 'h':
         case '?':
            showHelp(argv);
            exit(0);
            break;

         default:
            fprintf( stderr, "Invalid option %s\n", argv[optind] );
            showHelp(argv);
            exit(1);
            break;
      }
   }

   if (optind < argc)
   {
      xmlrpcServer = argv[optind++];
      printf("ready to send the request to %s\n", xmlrpcServer);
   }

   if (optind < argc)
   {
      fprintf(stderr, "Too many arguments: '%s'\n", argv[optind]);
      showHelp(argv);
      exit(1);
   }
}

int main(int argc, char* argv[])
{
    parseArgs(argc, argv);
    initLogger(argv);

    UtlString urlString = "https://" + UtlString(xmlrpcServer);
    Url url(urlString);
    url.setHostPort(HttpPort);

    // Delay for 5 minutes
//    printf("Wating for start of memcheck ...\n");
//    OsTask::delay(600000);
//    printf("Starting memcheck ...\n");

    // Test run
    // while (1)
    for (int i = 0; i < 10; i++)
    {
        XmlRpcRequest* request = new XmlRpcRequest(url, "addExtension");

        UtlString groupName("acd@pingtel.com");
        request->addParam(&groupName);

        UtlString extension("666@pingtel.com");
        request->addParam(&extension);

        printf("Sending %d ...", i);
        XmlRpcResponse response;
        if (!request->execute(response))
        {
           UtlString reason;
           int code;
           response.getFault(&code,reason);
           printf(" failed\n   %d %s\n", code, reason.data() );
        }
        else
        {
           printf(" ok\n");
        }

        delete request;
        request = NULL;

//        OsTask::delay(2000);
    }
//    printf("Done with memcheck ...\n");

//    while (1)
//    {
//        OsTask::delay(60000);
//    }

    return(1);
}


// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
   return 0;
}
