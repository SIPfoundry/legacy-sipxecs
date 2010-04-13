//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// System includes
#include <getopt.h>

#include "os/OsDefs.h"
#if defined(_WIN32)
#   include <windows.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#endif

// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlHashMapIterator.h>
#include <net/XmlRpcRequest.h>
#include <net/XmlRpcDispatch.h>
#include <os/OsSysLog.h>
#include <os/OsTask.h>
#include <os/OsConfigDb.h>

#include <configrpc/ConfigRPC.h>

// CONSTANTS
#define HTTP_PORT               8200    // Default HTTP port
#define MEMCHECK_DELAY          45
#define TEST_DATASET            "test.conf"
#define TEST_FILENAME           "configTest.db"
#define TEST_VERSION            "0.1"
#define TEST_LOGFILE            "xmlrpcServer.log"

enum Verbosity
{
   Quiet,
   Normal,
   Verbose
} Feedback = Quiet;

int MemCheckDelay = 0;
int HttpPort = HTTP_PORT;
int Duration = 30000;

class test_Callback : public ConfigRPC_Callback
{
   void modified()
   {
   }
};

void showHelp(char* argv[])
{
   printf("Usage: \n"
          "   %s:\n"
          "   [ {-v|--verbose} | {-q|--quiet} ]\n"
          "   [ {-d|--duration} <seconds> ]\n"
          "   [ {-p|--port} <port> ]\n"
          "   [ {-h|-?|--help} ]\n"
          , argv[0]
          );
}

void parseArgs(int argc, char* argv[])
{
   int optResult = 0;

   const char* short_options = "d:p:vqmh";

   const struct option long_options[] =
      {
         {"verbose", no_argument, NULL, 'v'},
         {"quiet",   no_argument, NULL, 'q'},
         {"memcheck",no_argument, &MemCheckDelay, MEMCHECK_DELAY},
         {"help",    no_argument, NULL, 'h'},
         {"port",    required_argument, NULL, 'p'},
         {"duration",required_argument, NULL, 'd'},
         {0, 0, 0, 0}
      };

   while (  (optResult = getopt_long (argc, argv, short_options, long_options, NULL))
          >= 0
          )
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

      case 'v':
         Feedback = Verbose;
         break;

      case 'q':
         Feedback = Quiet;
         break;

      case 'd':
         Duration = strtoul(optarg, &optend, 10);
         Duration *= 1000;
         if ( '\0' != *optend )
         {
            fprintf( stderr, "Invalid duration %s\n", optarg );
            exit(1);
         }
         break;

      case 'h':
      case '?':
         showHelp(argv);
         exit(0);
         break;

      default:
         break;
      }
   }
}

void initLogger(char* argv[])
{
    UtlString LogFile = TEST_LOGFILE;
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

int main(int argc, char *argv[])
{
    OsConfigDb configDb;
    UtlString configDbFile = TEST_FILENAME;

    parseArgs(argc, argv);

    if (MemCheckDelay)
    {
        // Delay 45 seconds to allow memcheck start
        printf("Wating %d seconds for start of memcheck ...", MemCheckDelay);
        OsTask::delay(MemCheckDelay * 1000);
        printf("starting\n");
    }

    initLogger(argv);

    // Reset database content an rewrite the file
    configDb.set("TestItem1", "Value1");
    configDb.set("TestItem2", "Value2");
    configDb.set("TestItem3", "Value3");
    configDb.set("TestItem3", "Value4");

    configDb.storeToFile(configDbFile);

    ConfigRPC_Callback* confCallbacks;
    ConfigRPC*          configRPC;
    XmlRpcDispatch*     rpc;

    // start a simple XmlRpc test server
    rpc = new XmlRpcDispatch(HTTP_PORT, false);

    confCallbacks = new test_Callback();
    configRPC     = new ConfigRPC( TEST_DATASET,
                                   TEST_VERSION,
                                   configDbFile,
                                   confCallbacks);
    // enter the connector RPC methods in the XmlRpcDispatch table
    ConfigRPC::registerMethods(*rpc);

    printf("Server will be up for %d seconds on port %d\n", Duration/1000, HttpPort);
    OsTask::delay(Duration);

    if (MemCheckDelay)
    {
        // Delay 45 seconds to allow memcheck start
        printf("Wating %d seconds for stop of memcheck ...", MemCheckDelay);
        OsTask::delay(MemCheckDelay * 1000);
        printf("starting\n");
    }

    exit(0);
}
