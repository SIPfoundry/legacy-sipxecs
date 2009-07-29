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
#include <sipXtapiDriver/SipLSendCommand.h>
#include <sipXtapiDriver/CommandProcessor.h>
#include <os/OsConnectionSocket.h>
#include <os/OsDatagramSocket.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipLSendCommand::SipLSendCommand()
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
SipLSendCommand::SipLSendCommand(const SipLSendCommand& rSipLSendCommand)
{
}

// Destructor
SipLSendCommand::~SipLSendCommand()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipLSendCommand&
SipLSendCommand::operator=(const SipLSendCommand& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

int SipLSendCommand::execute(int argc, char* argv[])
{
        int commandStatus = CommandProcessor::COMMAND_FAILED;
        UtlString messageBuffer;
        char buffer[1025];
        int bufferSize = 1024;
        int charsRead;

        printf("send command with %d arguments\n", argc);
        if(argc != 5)
        {
                UtlString usage;
                getUsage(argv[0], &usage);
                printf("%s", usage.data());
        }

        else
        {
        UtlString protocol(argv[2]);
        protocol.toUpper();

        UtlString hostAddress(argv[3]);
        int hostPort = atoi(argv[4]);

        FILE* sipMessageFile = fopen(argv[1], "r");
                if(sipMessageFile && hostPort > 0 && !hostAddress.isNull() &&
            (protocol.compareTo("TCP") == 0 || protocol.compareTo("UDP") == 0))
                {
                        //printf("opened file: \"%s\"\n", argv[1]);
                        do
                        {
                                charsRead = fread(buffer, 1, bufferSize, sipMessageFile);
                                if(charsRead > 0)
                                {
                                        messageBuffer.append(buffer, charsRead);
                                }
                        }
                        while(charsRead);
            fclose(sipMessageFile);

            OsSocket* writeSocket = NULL;
            if(protocol.compareTo("TCP") == 0)
                writeSocket = new OsConnectionSocket(hostPort ,hostAddress);
            else if(protocol.compareTo("UDP") == 0)
                writeSocket = new OsDatagramSocket(hostPort ,hostAddress);

                        //printf("Read file contents:\n%s\n====END====\n", messageBuffer.data());
                        //SipMessage message(messageBuffer.data());

            int bytesSent;
                        if((bytesSent = writeSocket->write(messageBuffer.data(),
                                                                                        messageBuffer.length())) > 0)
                        {
                                commandStatus = CommandProcessor::COMMAND_SUCCESS;
                        }
                        else
                        {
                                printf("Failed to send SIP message");
                        }
            printf("Send message with %d bytes\n", bytesSent);
                }
                else if(!sipMessageFile)
                {
                        printf("send file: \"%s\" does not exist\n", argv[1]);
                        commandStatus = CommandProcessor::COMMAND_FAILED;
                }
        else if(hostPort <= 0)
        {
                        printf("Invalid destination port: %s\n", argv[4]);
                        commandStatus = CommandProcessor::COMMAND_FAILED;
                }
        else if(hostAddress.isNull())
        {
                        printf("Invalid destination address: %s\n", argv[3]);
                        commandStatus = CommandProcessor::COMMAND_FAILED;
                }
        else if(protocol.compareTo("TCP")  && protocol.compareTo("UDP"))
        {
                        printf("Invalid protocol: %s\n", argv[2]);
                        commandStatus = CommandProcessor::COMMAND_FAILED;
                }

        }

        return(commandStatus);
}

/* ============================ ACCESSORS ================================= */

void SipLSendCommand::getUsage(const char* commandName, UtlString* usage) const
{
        Command::getUsage(commandName, usage);
        usage->append("<sipMesageFileName> <protocol> <host_address> <host_port>\nWhere:\n\tsipMesageFileName - is the file containing the message to be sent\n\tprotocol - is TCP or UDP\n\thost_address - is the host IP or DNS name to send the message to\n\thost_port - is the port on the destination host\n");
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool SipLSendCommand::sIsTested = false;

// Test this class by running all of its assertion tests
void SipLSendCommand::test()
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
void SipLSendCommand::testCreators()
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
void SipLSendCommand::testManipulators()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the accessor methods for the class
void SipLSendCommand::testAccessors()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void SipLSendCommand::testInquiry()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
