//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _HistoryCommand_h_
#define _HistoryCommand_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <sipXtapiDriver/Command.h>
#include <sipXtapiDriver/CommandProcessor.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class HistoryCommand : public Command
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   HistoryCommand(CommandProcessor* processor = NULL);
     //:Default constructor

   virtual
   ~HistoryCommand();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual int execute(int argc, char* argv[]);

/* ============================ ACCESSORS ================================= */

        virtual void getUsage(const char* commandName, UtlString* usage) const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        CommandProcessor* commandProcessor;

        HistoryCommand(const HistoryCommand& rHistoryCommand);
        //:Copy constructor
        HistoryCommand& operator=(const HistoryCommand& rhs);
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

#endif  // _HistoryCommand_h_
