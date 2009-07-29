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
#include <sipXtapiDriver/RespondCommand.h>
#include <sipXtapiDriver/CommandProcessor.h>
#include <sipXtapiDriver/CommandMsgProcessor.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
RespondCommand::RespondCommand(CommandMsgProcessor* processor)
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

   messageProcessor = processor;

}

// Copy constructor
RespondCommand::RespondCommand(const RespondCommand& rRespondCommand)
{
}

// Destructor
RespondCommand::~RespondCommand()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
RespondCommand&
RespondCommand::operator=(const RespondCommand& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

int RespondCommand::execute(int argc, char* argv[])
{
        int status = CommandProcessor::COMMAND_FAILED;
        UtlString usage;

        //printf("no operation command with %d arguments\n", argc);
        if(argc == 1)
        {
                status = CommandProcessor::COMMAND_SUCCESS;
                messageProcessor->stopResponding();
        }
        else if(argc >= 3)
        {
                int statusCode = atoi(argv[1]);
                if(statusCode <= 0)
                {
                        printf("status code: %d must be greater than zero", statusCode);
                        status = CommandProcessor::COMMAND_FAILED;
                }
                else
                {
                        int argIndex;
                        UtlString statusText;
                        for(argIndex = 2; argIndex < argc; argIndex++)
                        {
                                if(!statusText.isNull())
                                {
                                        statusText.append(' ');
                                }
                                statusText.append(argv[argIndex]);
                        }

                        messageProcessor->startResponding(statusCode, statusText.data(), -1);
                        status = CommandProcessor::COMMAND_SUCCESS;
                }

        }

        else
        {
                UtlString usage;
                getUsage(argv[0], &usage);
                printf("%s", usage.data());
                status = CommandProcessor::COMMAND_BAD_SYNTAX;
        }

        return(status);
}

void RespondCommand::getUsage(const char* commandName, UtlString* usage) const
{
        Command::getUsage(commandName, usage);
        usage->append(" [<statusCode> <statusText>]\n");

}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool RespondCommand::sIsTested = false;

// Test this class by running all of its assertion tests
void RespondCommand::test()
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
void RespondCommand::testCreators()
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
void RespondCommand::testManipulators()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the accessor methods for the class
void RespondCommand::testAccessors()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void RespondCommand::testInquiry()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
