//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES

#ifdef TEST
#include <assert.h>
#include "utl/UtlMemCheck.h"
#endif //TEST
#include <stdio.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include <siptest/SipLogCommand.h>
#include <net/SipUserAgent.h>
//#include <siptest/CommandProcessor.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipLogCommand::SipLogCommand(SipUserAgent& userAgent)
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

   mSipUserAgent = &userAgent;

}

// Copy constructor
SipLogCommand::SipLogCommand(const SipLogCommand& rSipLogCommand)
{
}

// Destructor
SipLogCommand::~SipLogCommand()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipLogCommand&
SipLogCommand::operator=(const SipLogCommand& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

int SipLogCommand::execute(int argc, char* argv[])
{
        //printf("no operation command with %d arguments\n", argc);
        //printf("exiting with status: %d\n", CommandProcessor::COMMAND_SUCCESS_EXIT);

        int commandStatus = CommandProcessor::COMMAND_FAILED;

        if(argc == 2)
        {
                UtlString logOperations(argv[1]);
        commandStatus = CommandProcessor::COMMAND_SUCCESS;

        if(logOperations.compareTo("stop") == 0)
        {
            mSipUserAgent->stopMessageLog();
            osPrintf("SIP logging Stopped\n");
        }

        else if(logOperations.compareTo("start") == 0)
        {
            mSipUserAgent->clearMessageLog();
            mSipUserAgent->startMessageLog();
            osPrintf("SIP logging Started\n");
        }

        else if(logOperations.compareTo("dump") == 0)
        {
            UtlString log;
            mSipUserAgent->getMessageLog(log);
            osPrintf("\n============>\n%s\n============>\n", log.data());
        }

        else if(logOperations.compareTo("clear") == 0)
        {
            mSipUserAgent->clearMessageLog();
            osPrintf("SIP log cleared\n");
        }

        else
        {
            argc = 1;
        }


        }

        if(argc != 2)
        {
                UtlString usage;
                getUsage(argv[0], &usage);
                printf("%s", usage.data());
                commandStatus = CommandProcessor::COMMAND_BAD_SYNTAX;
        }
        else
        {

        }

        return(commandStatus);
}

void SipLogCommand::getUsage(const char* commandName, UtlString* usage) const
{
        Command::getUsage(commandName, usage);
        usage->append(" stop|start|dump|clear\n");
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool SipLogCommand::sIsTested = false;

// Test this class by running all of its assertion tests
void SipLogCommand::test()
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
void SipLogCommand::testCreators()
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
void SipLogCommand::testManipulators()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the accessor methods for the class
void SipLogCommand::testAccessors()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void SipLogCommand::testInquiry()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
