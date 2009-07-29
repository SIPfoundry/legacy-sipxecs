//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _RespondCommand_h_
#define _RespondCommand_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <sipXtapiDriver/Command.h>
#include <sipXtapiDriver/CommandMsgProcessor.h>

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
class RespondCommand : public Command
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   RespondCommand(CommandMsgProcessor* processor = NULL);
     //:Default constructor

   virtual
   ~RespondCommand();
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
        CommandMsgProcessor* messageProcessor;

        RespondCommand(const RespondCommand& rRespondCommand);
        //:Copy constructor
        RespondCommand& operator=(const RespondCommand& rhs);
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

#endif  // _RespondCommand_h_
