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
#include <stdlib.h>

// APPLICATION INCLUDES
#include <sipXtapiDriver/HistoryCommand.h>
#include <sipXtapiDriver/CommandProcessor.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
HistoryCommand::HistoryCommand(CommandProcessor* processor)
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

   commandProcessor = processor;

}

// Copy constructor
HistoryCommand::HistoryCommand(const HistoryCommand& rHistoryCommand)
{
}

// Destructor
HistoryCommand::~HistoryCommand()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
HistoryCommand&
HistoryCommand::operator=(const HistoryCommand& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

int HistoryCommand::execute(int argc, char* argv[])
{
        //printf("no operation command with %d arguments\n", argc);
        //printf("exiting with status: %d\n", CommandProcessor::COMMAND_SUCCESS_EXIT);

        int commandStatus = CommandProcessor::COMMAND_FAILED;
        int historyLength = commandProcessor->getHistoryLength();
        int requestedLength = historyLength;
        int historyIndex;

        if(argc == 2)
        {
                requestedLength = atoi(argv[1]);
        }

        if(argc > 2 || requestedLength < 1)
        {
                UtlString usage;
                getUsage(argv[0], &usage);
                printf("%s", usage.data());
                commandStatus = CommandProcessor::COMMAND_BAD_SYNTAX;
        }
        else
        {
                UtlString commandLine;

                if(historyLength < requestedLength)
                {
                        requestedLength = historyLength;
                }

                for(historyIndex = historyLength - requestedLength;
                historyIndex < historyLength;
                historyIndex++)
                {
                        commandProcessor->getHistory(historyIndex, &commandLine);
                        printf("%d: %s\n", historyIndex, commandLine.data());
                }
                commandStatus = CommandProcessor::COMMAND_SUCCESS;
        }

        return(commandStatus);
}

void HistoryCommand::getUsage(const char* commandName, UtlString* usage) const
{
        Command::getUsage(commandName, usage);
        usage->append(" [lastNCommands]\n");
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool HistoryCommand::sIsTested = false;

// Test this class by running all of its assertion tests
void HistoryCommand::test()
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
void HistoryCommand::testCreators()
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
void HistoryCommand::testManipulators()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the accessor methods for the class
void HistoryCommand::testAccessors()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void HistoryCommand::testInquiry()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
