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
#include <sipXtapiDriver/SipSendCommand.h>
#include <sipXtapiDriver/CommandProcessor.h>
#include <os/OsDateTime.h>
#include <os/OsTime.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
static const
char UsageMsg[] = "<sipMessageFileName> [numberOfTransactions]\n";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipSendCommand::SipSendCommand(SipUserAgent* userAgent)
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

   sipUserAgent = userAgent;
}

// Copy constructor
SipSendCommand::SipSendCommand(const SipSendCommand& rSipSendCommand)
{
}

// Destructor
SipSendCommand::~SipSendCommand()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipSendCommand&
SipSendCommand::operator=(const SipSendCommand& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

int SipSendCommand::execute(int argc, char* argv[])
{
        int commandStatus = CommandProcessor::COMMAND_FAILED;
        UtlString messageBuffer;
        char buffer[1025];
        int bufferSize = 1024;
        int charsRead;

        if(argc != 2 && argc != 3)
        {
                printf("Usage: %s %s", argv[0], UsageMsg);
        }

        else
        {
        int transactionCount = 1;
        if(argc == 3) transactionCount = atoi(argv[2]);

        FILE* sipMessageFile = fopen(argv[1], "r");
                if(sipMessageFile)
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

                        //printf("Read file contents:\n%s\n====END====\n", messageBuffer.data());
                        SipMessage message(messageBuffer.data());
            UtlString callId;
            message.getCallIdField(&callId);
            char callIdBuffer[500];
            OsTime start;
            OsDateTime::getCurTimeSinceBoot(start);
            int transactionIndex;
            for(transactionIndex = 0; transactionIndex < transactionCount;
            transactionIndex++)
            {
                            if(sipUserAgent->send(message))
                            {
                                    commandStatus = CommandProcessor::COMMAND_SUCCESS;
                            }
                            else
                            {
                                    printf("Failed to send SIP message");
                    break;
                            }
                sprintf(callIdBuffer, "%d-%s", transactionIndex + 1,
                    callId.data());
                message.setCallIdField(callIdBuffer);
            }
            OsTime finish;
            OsDateTime::getCurTimeSinceBoot(finish);
            OsTime lapse = finish - start;
            double seconds = lapse.seconds() +
                ((double) lapse.usecs())/1000000.0;
            double transPerSecond = ((double) transactionIndex) / seconds;
            printf("Transactions: %d Seconds: %f tps: %f spt: %f\n",
                transactionIndex, seconds, transPerSecond,
                1.0/transPerSecond);

                }
                else
                {
                        printf("send file: \"%s\" does not exist\n", argv[1]);
                        commandStatus = CommandProcessor::COMMAND_FAILED;
                }
        }

        return(commandStatus);
}

/* ============================ ACCESSORS ================================= */

void SipSendCommand::getUsage(const char* commandName, UtlString* usage) const
{
        Command::getUsage(commandName, usage);
        usage->append( UsageMsg );
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool SipSendCommand::sIsTested = false;

// Test this class by running all of its assertion tests
void SipSendCommand::test()
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
void SipSendCommand::testCreators()
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
void SipSendCommand::testManipulators()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the accessor methods for the class
void SipSendCommand::testAccessors()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void SipSendCommand::testInquiry()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
