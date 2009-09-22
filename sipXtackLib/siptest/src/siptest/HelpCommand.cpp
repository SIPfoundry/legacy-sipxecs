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

// APPLICATION INCLUDES
#include <siptest/HelpCommand.h>
#include <siptest/CommandProcessor.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
HelpCommand::HelpCommand(CommandProcessor* processor)
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
HelpCommand::HelpCommand(const HelpCommand& rHelpCommand)
{
}

// Destructor
HelpCommand::~HelpCommand()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
HelpCommand&
HelpCommand::operator=(const HelpCommand& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

int HelpCommand::execute(int argc, char* argv[])
{
        int status = CommandProcessor::COMMAND_FAILED;
        UtlString usage;

        //printf("no operation command with %d arguments\n", argc);
        if(argc == 1)
        {
                status = CommandProcessor::COMMAND_SUCCESS;
                getUsage(argv[0], &usage);
                printf(usage.data());
        }
        else if(argc == 2)
        {
                Command* command;

                if(commandProcessor &&
                        commandProcessor->findCommand(argv[1], &command) ==
                        CommandProcessor::COMMAND_SUCCESS)
                {
                        if(command)
                        {
                                //printf("found help for: \"%s\" command\n", argv[1]);
                                status = CommandProcessor::COMMAND_SUCCESS;
                                command->getUsage(argv[1], &usage);
                                printf(usage.data());
                        }
                }
                if(status != CommandProcessor::COMMAND_SUCCESS)
                {
                        status = CommandProcessor::COMMAND_NOT_FOUND;
                        printf("%s command: \"%s\" not found\n", argv[0], argv[1]);
                }
        }
        else
        {
                status = CommandProcessor::COMMAND_BAD_SYNTAX;
        }

        return(status);
}

void HelpCommand::getUsage(const char* commandName, UtlString* usage) const
{
        Command::getUsage(commandName, usage);
        usage->append(" <commandName>\n");

        if(commandProcessor)
        {
                int numCommands = commandProcessor->getNumCommands();
                int commandIndex;
                UtlString commandNameString;

                if(numCommands > 0)
                {
                        usage->append("Where commandName may be one of the following:\n");

                        for(commandIndex = 0; commandIndex < numCommands; commandIndex++)
                        {
                                commandProcessor->getCommandName(commandIndex, &commandNameString);
                                {
                                        if(!commandNameString.isNull())
                                        {
                                                if(commandIndex > 0)
                                                {
                                                        usage->append(' ');
                                                }
                                                usage->append(commandNameString.data());
                                        }
                                }
                        }
                        usage->append('\n');
                }
        }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool HelpCommand::sIsTested = false;

// Test this class by running all of its assertion tests
void HelpCommand::test()
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
void HelpCommand::testCreators()
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
void HelpCommand::testManipulators()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the accessor methods for the class
void HelpCommand::testAccessors()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void HelpCommand::testInquiry()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
