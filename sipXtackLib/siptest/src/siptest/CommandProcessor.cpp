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
#include <siptest/CommandProcessor.h>
#include <net/NameValueTokenizer.h>
#include <utl/UtlTokenizer.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CommandProcessor::CommandProcessor()
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

   numCommands = 0;
}

// Copy constructor
CommandProcessor::CommandProcessor(const CommandProcessor& rCommandProcessor)
{
}

// Destructor
CommandProcessor::~CommandProcessor()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
CommandProcessor&
CommandProcessor::operator=(const CommandProcessor& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void CommandProcessor::registerCommand(const char* commandName, Command* command)
{
        if(numCommands < MAX_COMMANDS)
        {
                (commandNames[numCommands]).remove(0);
                (commandNames[numCommands]).append(commandName);
                commands[numCommands] = command;
                numCommands++;
        }
}

int CommandProcessor::executeCommand(const char* commandLine)
{
        int argc;
        char** argv = NULL;
        int returnStatus = COMMAND_FAILED;
        int argIndex;

        parseCommandLine(commandLine, &argc, &argv);
        //printf("found %d arguments\n", argc);

        if(argc && argv)
        {
                returnStatus = executeCommand(argc, argv);
                //printf("CommandProcessor::executeCommand(const char* commandLine) exiting with status: %d\n", returnStatus);

                if(returnStatus == COMMAND_NOT_FOUND)
                {
                        printf("Command: \"%s\" not found\n", argv[0]);
                }

                for(argIndex = 0; argIndex < argc; argIndex++)
                {
                        delete[] argv[argIndex];
                }
                delete[] argv;

                pushHistory(commandLine);

        }

        //printf("CommandProcessor::executeCommand(const char* commandLine) exiting with status: %d\n", returnStatus);

        return(returnStatus);
}

int CommandProcessor::executeCommand(int argc, char* argv[])
{
        int returnStatus = COMMAND_FAILED;
        Command* command;

        returnStatus = findCommand(argv[0], &command);
        if(returnStatus == COMMAND_SUCCESS && command)
        {
                returnStatus = command->execute(argc, argv);
        }

        return(returnStatus);
}

void CommandProcessor::pushHistory(const char* commandLine)
{
        historyList.append(new UtlString(commandLine));
}

UtlBoolean CommandProcessor::getHistory(int index, UtlString* commandLine) const
{
        UtlString* historyCommand = NULL;
        commandLine->remove(0);

        if(index >= 0)
        {
                historyCommand = (UtlString*) historyList.at(index);
                if(historyCommand)
                {
                        commandLine->append(historyCommand->data());
                }
        }
        return(historyCommand != NULL);
}

int CommandProcessor::getHistoryLength() const
{
        return(historyList.entries());
}

int CommandProcessor::findCommand(const char* commandName, Command** command) const
{
        int status = COMMAND_NOT_FOUND;
        int commandIndex;

        *command = NULL;

        for(commandIndex = 0; commandIndex < numCommands; commandIndex++)
        {
                //printf("Comparing: command: \'%s\" with Command: \"%s\"\n", commandName,
                //      (commandNames[commandIndex]).data());

                if(strstr((commandNames[commandIndex]).data(), commandName) ==
                        (commandNames[commandIndex]).data())
                {
                        if(*command)
                        {
                                printf("multiple commands found\n");
                                status = COMMAND_AMBIGUOUS;
                                break;
                        }
                        else
                        {
                                //printf("command found\n");
                                *command = commands[commandIndex];
                                status = COMMAND_SUCCESS;
                        }
                }
        }
        return(status);
}

void CommandProcessor::parseCommandLine(const char* commandLine, int* argc, char*** argv)
{
        UtlString arg;
        int argIndex = 0;

        // Hard code length for now;
        *argv = new char*[1024];
        *argc = 0;

        if(commandLine)
        {
                //printf("creating tokenizer\n");
                UtlTokenizer tokenizer(commandLine);

                do
                {
                        arg.remove(0);
                        tokenizer.next(arg, " \t\n");

                        //printf("arg[%d]=\"%s\"\n", argIndex, arg.data());
                        if(!arg.isNull())
                        {
                                (*argv)[argIndex] = new char[strlen(arg.data()) + 1];
                                strcpy((*argv)[argIndex], arg.data());

                                argIndex++;
                        }

                }
                while(!arg.isNull());
        }
        *argc = argIndex;
}

int CommandProcessor::getNumCommands() const
{
        return(numCommands);
}

void CommandProcessor::getCommandName(int index, UtlString* commandName) const
{
        if(index >= 0 && index < numCommands)
        {
                commandName->remove(0);
                commandName->append((commandNames[index]).data());
        }
}

/* ============================ ACCESSORS ================================= */

void CommandProcessor::setEnvironment(const char* name, const char* value)
{
    mEnvironment.set(name, value);
}

UtlBoolean CommandProcessor::getEnvironment(const char* name, UtlString& value)
{
    return(OS_SUCCESS == mEnvironment.get(name, value));
}

UtlBoolean CommandProcessor::getEnvironment(const char* name, int& value)
{
    return(OS_SUCCESS == mEnvironment.get(name, value));
}
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool CommandProcessor::sIsTested = false;

// Test this class by running all of its assertion tests
void CommandProcessor::test()
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
void CommandProcessor::testCreators()
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
void CommandProcessor::testManipulators()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the accessor methods for the class
void CommandProcessor::testAccessors()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void CommandProcessor::testInquiry()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
