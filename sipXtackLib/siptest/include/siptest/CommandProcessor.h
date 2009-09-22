//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _CommandProcessor_h_
#define _CommandProcessor_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlDList.h>
#include <utl/UtlDListIterator.h>
#include <siptest/Command.h>
#include <os/OsDefs.h>
#include <os/OsConfigDb.h>



// DEFINES
#define MAX_COMMANDS 1024

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CommandProcessor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

        enum CommandStatus
        {
                COMMAND_SUCCESS = 0,
                COMMAND_SUCCESS_EXIT,
                COMMAND_FAILED = 100,
                COMMAND_NOT_FOUND,
                COMMAND_AMBIGUOUS,
                COMMAND_BAD_SYNTAX,
                COMMAND_FAILED_EXIT = 200
        };

/* ============================ CREATORS ================================== */

   CommandProcessor();
     //:Default constructor


   virtual
   ~CommandProcessor();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

void registerCommand(const char* commandName, Command* command);

int executeCommand(const char* commandLine);

/* ============================ ACCESSORS ================================= */

int findCommand(const char* commandName, Command** command) const;

static void parseCommandLine(const char* commandLine, int* argc, char*** argv);

int getNumCommands() const;

void getCommandName(int commandIndex, UtlString* commandNameString) const;

int getHistoryLength() const;

UtlBoolean getHistory(int index, UtlString* commandLine) const;

void setEnvironment(const char* name, const char* value);
UtlBoolean getEnvironment(const char* name, UtlString& value);
UtlBoolean getEnvironment(const char* name, int& value);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
int executeCommand(int argc, char* argv[]);
void pushHistory(const char* commandLine);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

        int numCommands;
        UtlString commandNames[MAX_COMMANDS];
        Command* commands[MAX_COMMANDS];
        UtlDList historyList;
    OsConfigDb mEnvironment;

   CommandProcessor(const CommandProcessor& rCommandProcessor);
     //:Copy constructor
   CommandProcessor& operator=(const CommandProcessor& rhs);
     //:Assignment operator

#ifdef TEST
   static bool sIsTested;
     //:Set to true after the tests for this class have been executed once

   void test();
     //:Verify assertions for this class

   // Test helper functions
   void testCreators();
   void testManipulators();
   void testAccessors();
   void testInquiry();

#endif //TEST
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CommandProcessor_h_
