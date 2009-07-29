//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#ifdef TEST
#include <assert.h>
#include "utl/UtlMemCheck.h"
#endif //TEST
#include <stdio.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include <sipXtapiDriver/HttpGetCommand.h>
#include <sipXtapiDriver/CommandProcessor.h>
#include <net/HttpMessage.h>
#include <os/OsConnectionSocket.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
HttpGetCommand::HttpGetCommand()
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

}

// Copy constructor
HttpGetCommand::HttpGetCommand(const HttpGetCommand& rHttpGetCommand)
{
}

// Destructor
HttpGetCommand::~HttpGetCommand()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
HttpGetCommand&
HttpGetCommand::operator=(const HttpGetCommand& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

int HttpGetCommand::execute(int argc, char* argv[])
{
    int commandStatus = CommandProcessor::COMMAND_FAILED;
    if(argc == 2)
    {
        commandStatus = CommandProcessor::COMMAND_SUCCESS;
        const char* url = argv[1];
        const char* serverBegin = strstr(url, "http://");
        if(serverBegin != url)
        {
            printf("unsupported protocol in Url: %s\n",
                url);
                    commandStatus = CommandProcessor::COMMAND_BAD_SYNTAX;
        }

        else
        {
            serverBegin += 7;
            UtlString uri(serverBegin);
            int serverEndIndex = uri.index("/");
            if(serverEndIndex < 0) serverEndIndex = uri.length();
            if(serverEndIndex > 0)
            {
                UtlString server = uri;
                server.remove(serverEndIndex);
                int portIndex = server.index(":");
                int port = 0;
                if(portIndex > 0)
                {
                    UtlString portString = server;
                    server.remove(portIndex);
                    portString.remove(0, portIndex + 1);
                    printf("port string: %s\n", portString.data());
                    port = atoi(portString.data());
                }
                uri.remove(0, serverEndIndex);
                if(uri.isNull()) uri = "/";
                printf("HTTP get of %s from server %s port: %d\n",
                    uri.data(), server.data(), port);

                if(port == 0)
                {
                    port = 80;
                    printf("defaulting to http port 80\n");
                }

                OsConnectionSocket getSocket(port, server.data());
                HttpMessage getRequest;
                getRequest.setFirstHeaderLine("GET", uri.data(), HTTP_PROTOCOL_VERSION);

                int wroteBytes = getRequest.write(&getSocket);
                printf("wrote %d\n", wroteBytes);

                HttpMessage getResponse;
                getResponse.read(&getSocket);

                UtlString responseBytes;
                int responseLength;
                getResponse.getBytes(&responseBytes, &responseLength);
                printf("Got %d bytes\n", responseLength);
                printf("Response: ++++++++++++++++++++++++++++++++++\n%s\n",
                    responseBytes.data());
            }

            else
            {
                printf("invalid server in Url: %s\n",
                url);
                        commandStatus = CommandProcessor::COMMAND_BAD_SYNTAX;
            }
        }
    }

        else
        {
                UtlString usage;
                getUsage(argv[0], &usage);
                printf("%s", usage.data());
                commandStatus = CommandProcessor::COMMAND_BAD_SYNTAX;
                //commandStatus = CommandProcessor::COMMAND_FAILED;
        }

        return(commandStatus);
}

/* ============================ ACCESSORS ================================= */

void HttpGetCommand::getUsage(const char* commandName, UtlString* usage) const
{
        Command::getUsage(commandName, usage);
        usage->append("<http-url>\n(i.e. http://10.1.1.100/index.html)\n");
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool HttpGetCommand::sIsTested = false;

// Test this class by running all of its assertion tests
void HttpGetCommand::test()
{

   UtlMemCheck* pUtlMemCheck = 0;
   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   testCreators();
   testManipulators();
   testAccessors();
   testInquiry();

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the creators (and destructor) methods for the class
void HttpGetCommand::testCreators()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the default constructor (if implemented)
   // test the copy constructor (if implemented)
   // test other constructors (if implemented)
   //    if a constructor parameter is used to set information in an ancestor
   //       class, then verify it gets set correctly (i.e., via ancestor
   //       class accessor method.
   // test the destructor
   //    if the class contains member pointer variables, verify that the
   //    pointers are getting scrubbed.

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the manipulator methods
void HttpGetCommand::testManipulators()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the accessor methods for the class
void HttpGetCommand::testAccessors()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void HttpGetCommand::testInquiry()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
