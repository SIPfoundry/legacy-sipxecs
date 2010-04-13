//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
#ifndef _SleepCommand_h_
#define _SleepCommand_h_
#include <stdio.h>
#include <assert.h>
#if defined(_WIN32)
# include <io.h>
# define STDIN_FILENO 0 /* can't find where windows defines this */
#else
# include <unistd.h>
#endif

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <sipXtapiDriver/CommandProcessor.h>
#include <net/SipUserAgent.h>
#include <sipXtapiDriver/ExitCommand.h>

class SleepCommand : public Command
{
public:
   SleepCommand(){};
   //:Default constructor

   virtual
   ~SleepCommand(){};
   //:Destructor

   virtual int execute(int argc, char* argv[]);

   virtual void getUsage(const char* commandName, UtlString* usage) const;
private:

   SleepCommand(const ExitCommand& rExitCommand);
   //:Copy constructor
   SleepCommand& operator=(const ExitCommand& rhs);
   //:Assignment operator
};
#endif //_SleepCommand_h_
