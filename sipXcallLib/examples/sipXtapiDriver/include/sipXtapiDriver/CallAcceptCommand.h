//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _CallAcceptCommand_h_
#define _CallAcceptCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/Command.h>
#include <sipXtapiDriver/CommandProcessor.h>

class CallAcceptCommand : public Command
{
public:
	//Constructor
	CallAcceptCommand();
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;
private:
};
#endif //_CallAcceptCommand_h_
