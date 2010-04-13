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
#include <os/OsSysLog.h>
#include <os/OsTask.h>
#include <os/OsEvent.h>
#include <net/HttpConnectionMap.h>

// CONSTANTS
#define MEMCHECK_DELAY 45
#define HTTP_PORT               8200    // Default HTTP port

char InputFile[128];
void fileExecute(const char *, bool);

// Define a client task for multi-threaded requests /////////////////////////////////////
class ClientTask : public OsTask
{
public:
    ClientTask(void* pArg);
    virtual int run(void* runArg);
};

ClientTask::ClientTask(void* pArg) : OsTask("xmlClientTask-%d", pArg)
{
}

int ClientTask::run(void* runArg)
{
    OsEvent* pEvent = (OsEvent*)runArg;
    OsStatus status;

    fileExecute(InputFile, false);

    do {
        //printf("%s is signaling\n", mName.data());
        status = pEvent->signal(1);
    }
    while (status == OS_ALREADY_SIGNALED);

    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////
int HttpPort = HTTP_PORT;
enum Verbosity
{
   Quiet,
   Normal,
   Verbose
} Feedback = Quiet;

const char* LogFile = "xmlrpcClient.log";
const char* xmlrpcURI;
int MemCheckDelay = 0;

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
   printf("Usage: \n"
          "   %s:\n"
          "   [ {-v|--verbose} | {-q|--quiet} ]\n"
          "   [ {-h|-?|--help} ]\n"
          "   [ --version <xmlrpc URI> <dataset> ]\n"
          "   [ {-g|--get} <xmlrpc URI> <dataset> <name> ... ]\n"
          "   [ {-s|--set} <xmlrpc URI> <dataset> <name> <value> [ <name> <value> ] ... ]\n"
          "   [ {-d|--delete} <xmlrpc URI> <dataset> <name> ... ]\n"
          "   [ {-f|--file} <file name> ]\n"
          "   [ {-t|--threads} <number> ]\n"
          , argv[0]
          );
}

typedef enum
{
   Version,         ///< configurationParameter.version
   Get,             ///< configurationParameter.get
   Set,             ///< configurationParameter.set
   Delete           ///< configurationParameter.delete
} MethodType;

MethodType Method;
UtlString  DataSet;
bool bInputFile = false;
bool bSingleStep = false;
bool bRepeatFile = false;
bool bThreads = false;
int numThreads = 1;

void parseArgs(int argc, char* argv[])
{
   int optResult = 0;
   int temp = 0;

   const char* short_options = "p:l:f:t:vqmhgsr";

   const struct option long_options[] =
      {
         {"verbose", no_argument, NULL, 'v'},
         {"quiet",   no_argument, NULL, 'q'},
         {"memcheck",no_argument, &MemCheckDelay, MEMCHECK_DELAY},
         {"help",    no_argument, NULL, 'h'},
         {"port",    required_argument, NULL, 'p'},
         {"log",     required_argument, NULL, 'l'},
         {"file",    required_argument, NULL, 'f'},
         {"threads", required_argument, NULL, 't'},
         {"step",    no_argument, NULL, 's'},
         {"repeat",  no_argument, NULL, 'r'},
         {"version", no_argument, (int*)&Method, Version },
         {"get",     no_argument, (int*)&Method, Get },
         {"set",     no_argument, (int*)&Method, Set },
         {"delete",  no_argument, (int*)&Method, Delete },
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

      case 'l':
         LogFile = optarg;
         break;

      case 'f':
         strncpy(InputFile, (char*)optarg, 128);
         bInputFile = true;
         break;

      case 't':
         temp = atoi((char*)optarg);
         numThreads = (temp > 0) ? temp : 1;
         break;

      case 's':
      {
         bSingleStep = true;
         break;
      }

      case 'r':
         bRepeatFile = true;
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
         break;
      }
   }

   if (optind < argc)
   {
      xmlrpcURI = argv[optind++];
   }

   if (optind < argc)
   {
      DataSet = argv[optind++];
   }
}

void exitFault(XmlRpcResponse& response)
{
   UtlString reason;
   int code;
   response.getFault(&code,reason);
   fprintf(stderr, "XML-RPC Fault %d: %s\n", code, reason.data() );
   //exit(1);
}

void requestVersion(Url& url)
{
    XmlRpcRequest* request;
    XmlRpcResponse response;

    request = new XmlRpcRequest(url, "configurationParameter.version");
    request->addParam(&DataSet);

    if (!request->execute(response/*, &pSocket*/))
    {
        exitFault(response);
    }
    else
    {
        UtlContainable* value;
        if (response.getResponse(value))
        {
            UtlString* versionId = dynamic_cast<UtlString*>(value);
            if (versionId)
            {
                printf("%s\n", versionId->data());
            }
            else
            {
                fprintf(stderr, "Incorrect type returned.\n");
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "No value returned.\n");
        }
    }
    delete request;
    request = NULL;
}

void requestGet(Url& url, UtlSList& names)
{
    XmlRpcRequest* request;
    XmlRpcResponse response;

    request = new XmlRpcRequest(url, "configurationParameter.get");
    request->addParam(&DataSet);

    if (!names.isEmpty())
    {
        request->addParam(&names);
    }

    if (!request->execute(response/*, &pSocket*/))
    {
        exitFault(response);
    }
    else
    {
        UtlContainable* value;
        if (response.getResponse(value))
        {
            UtlHashMap* paramList = dynamic_cast<UtlHashMap*>(value);
            if (paramList)
            {
                UtlHashMapIterator params(*paramList);
                UtlString* name;
                while ((name = dynamic_cast<UtlString*>(params())))
                {
                    UtlString* value = dynamic_cast<UtlString*>(paramList->findValue(name));
                    printf("%s : %s\n", name->data(), value->data());
                }
            }
            else
            {
                fprintf(stderr, "Incorrect type returned.\n");
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "No value returned.\n");
            exit(1);
        }
    }
    delete request;
    request = NULL;
}

void requestSet(Url& url, UtlHashMap& parameters)
{
    XmlRpcRequest* request;
    XmlRpcResponse response;

    request = new XmlRpcRequest(url, "configurationParameter.set");
    request->addParam(&DataSet);
    request->addParam(&parameters);

    if (request->execute(response /*, &pSocket*/))
    {
        UtlContainable* value;
        if (response.getResponse(value))
        {
            UtlInt* numberSet = dynamic_cast<UtlInt*>(value);
            if (numberSet)
            {
                if (Verbose == Feedback)
                {
                    printf("set %d name/value pairs.\n", (int)numberSet->getValue());
                }
            }
            else
            {
                fprintf(stderr, "Incorrect type returned.\n");
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "No value returned.\n");
            exit(1);
         }
    }
    else
    {
        exitFault(response);
    }
    delete request;
    request = NULL;
}

void requestDelete(Url& url, UtlSList& names)
{
    XmlRpcRequest* request;
    XmlRpcResponse response;

    request = new XmlRpcRequest(url, "configurationParameter.delete");
    request->addParam(&DataSet);

    if (!names.isEmpty())
    {
        request->addParam(&names);
    }

    if (!request->execute(response/*, &pSocket*/))
    {
        exitFault(response);
    }
    else
    {
        UtlContainable* value;
        if (response.getResponse(value))
        {
            UtlInt* deletedCount = dynamic_cast<UtlInt*>(value);
            if (deletedCount)
            {
                if (Verbose == Feedback)
                {
                   printf("deleted %d parameters.\n", (int)deletedCount->getValue());
                }
            }
            else
            {
                fprintf(stderr, "Incorrect type returned.\n");
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "No value returned.\n");
            exit(1);
        }
    }
    delete request;
    request = NULL;
}

int main(int argc, char* argv[])
{
   parseArgs(argc, argv);
   initLogger(argv);
   OsEvent taskDone;

   Url url(xmlrpcURI);

   if (MemCheckDelay)
   {
      // Delay 45 seconds to allow memcheck start
      printf("Wating %d seconds for start of memcheck ...", MemCheckDelay);
      OsTask::delay(MemCheckDelay * 1000);
      printf("starting\n");
   }

   // If an input file was specified we start up the number
   // of specified threads to execute that input file. If number
   // of threads wasn't specified we start up 1 thread.
   if (bInputFile)
   {
      int signaled = 0;

      for (int i=0; i<numThreads; i++)
      {
         ClientTask* pTask = new ClientTask(&taskDone);
         pTask->start();
      }

      // Wait for threads to shut down
      while (signaled < numThreads)
      {
         taskDone.wait();
         taskDone.reset();
         ++signaled;
      }
      exit(0);
   }

   switch (Method)
   {
   case Version: // --version <xmlrpc URI> <dataset>
   {
      if (optind < argc)
      {
         fprintf(stderr, "Too many arguments: '%s'\n", argv[optind]);
         showHelp(argv);
         exit(1);
      }

      requestVersion(url);

      break;
   }
   case Get: // --get <xmlrpc URI> <dataset> <name> ...
   {
      UtlSList names;
      // copy remaining arguments into the names list
      while (optind < argc)
      {
         names.append(new UtlString(argv[optind++]));
      }

      requestGet(url, names);

      break;
   }
   case Set: // --set <xmlrpc URI> <dataset> <name> <value> [ <name> <value> ] ...
   {
      UtlHashMap parameters;
      // copy remaining arguments into the names list
      while (optind + 1 < argc)
      {
         UtlString* setName = new UtlString(argv[optind++]);
         UtlString* setValue = new UtlString(argv[optind++]);
         parameters.insertKeyAndValue(setName, setValue);
      }
      if (optind < argc)
      {
         fprintf(stderr, "name '%s' without a value\n", argv[optind]);
         showHelp(argv);
         exit(1);
      }

      if (parameters.isEmpty())
      {
         fprintf(stderr, "must specify at least one name and value\n");
         showHelp(argv);
         exit(1);
      }
      else
      {
        requestSet(url, parameters);
        parameters.destroyAll();
      }

      break;
   }
   case Delete: // --delete <xmlrpc URI> <dataset> <name> ...
   {
      UtlSList names;
      // copy remaining arguments into the names list
      while (optind < argc)
      {
         names.append(new UtlString(argv[optind++]));
      }

      requestDelete(url, names);

      break;
   }
   default:
      fprintf(stderr, "No method specified\n");
      showHelp(argv);
      exit(1);
   }

   if (MemCheckDelay)
   {
      // Delay 45 seconds to allow memcheck start
      printf("Wating %d seconds for stop of memcheck ...", MemCheckDelay);
      OsTask::delay(MemCheckDelay * 1000);
      printf("starting\n");
   }

   exit(0);

}

void fileError(int error, int line)
{
    switch (error)
    {
        case 1:
            fprintf(stderr, "Expected URI in line %d - ignoring line\n", line);
            break;
        case 2:
            fprintf(stderr, "Expected data set in line %d - ignoring line\n", line);
            break;
        case 3:
            fprintf(stderr, "Name without a value specified in line %d - ignoring line\n", line);
            break;
        default:
            fprintf(stderr, "Unknown error encountered in line %d - ignoring line\n", line);
            break;
    }
}

void fileExecute(const char* inputFile, bool bSingleStep)
{
    FILE *fp;
    char szBuffer[128];
    char* token;
    int line = 0;

    if ((fp=fopen(inputFile, "r")) != NULL)
    {
        do {
            rewind(fp);
            while (fgets(szBuffer, 128, fp) != NULL)
            {
                ++line;
                if (szBuffer[0] != 0)
                {
                    printf("Executing %s", szBuffer);
                }
                token = strtok(szBuffer, " ");
                if (token == NULL)
                {
                    break;
                }
                if (strcasecmp(token, "version") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        fileError(1, line);
                    }
                    else
                    {
                        Url url(token);
                        token = strtok(NULL, " ");
                        if (token == NULL)
                        {
                            fileError(2, line);
                        }
                        else
                        {
                            DataSet = token;

                            requestVersion(url);
                        }
                    }
                }
                else if (strcasecmp(token, "get") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        fileError(1, line);
                    }
                    else
                    {
                        Url url(token);
                        token = strtok(NULL, " ");
                        if (token == NULL)
                        {
                            fileError(2, line);
                        }
                        else
                        {
                            DataSet = token;
                            UtlSList names;
                            while (token != NULL)
                            {
                                token = strtok(NULL, " ");
                                if (token != NULL)
                                {
                                    names.append(new UtlString(token));
                                }
                            }
                            requestGet(url, names);
                            names.destroyAll();
                        }
                    }
                }
                else if (strcasecmp(token, "set") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        fileError(1, line);
                    }
                    else
                    {
                        Url url(token);
                        token = strtok(NULL, " ");
                        if (token == NULL)
                        {
                            fileError(2, line);
                        }
                        else
                        {
                            DataSet = token;
                            UtlHashMap parameters;
                            char *key;
                            char *value;
                            while (token != NULL)
                            {
                                key = strtok(NULL, " ");
                                if (key == NULL)
                                {
                                    break;
                                }
                                value = strtok(NULL, " ");
                                if (value == NULL)
                                {
                                    fileError(3, line);
                                    break;
                                }
                                parameters.insertKeyAndValue(new UtlString(key), new UtlString(value));
                            }
                            int entries = parameters.entries();

                            if (entries != 0 || (entries%2) == 0)
                            {
                                requestSet(url, parameters);
                                parameters.destroyAll();
                            }
                        }
                    }
                }
                else if (strcasecmp(token, "delete") == 0)
                {
                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        fileError(1, line);
                    }
                    else
                    {
                        Url url(token);
                        token = strtok(NULL, " ");
                        if (token == NULL)
                        {
                            fileError(2, line);
                        }
                        else
                        {
                            DataSet = token;
                            UtlSList names;
                            while (token != NULL)
                            {
                                token = strtok(NULL, " ");
                                if (token != NULL)
                                {
                                    names.append(new UtlString(token));
                                }
                            }
                            requestDelete(url, names);
                            names.destroyAll();
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "Unknown RPC request %s - ignoring line\n", token);
                }
                if (bSingleStep)
                {
                    getchar();
                }
            }
        }
        while ( bRepeatFile );
        fclose(fp);
    }
    else
    {
        fprintf(stderr, "Can't open %s\n", inputFile);
        exit(1);
    }
}

// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
   return 0;
}
