//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _EnableRportCommand_h_
#define _EnableRportCommand_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class EnableRportCommand : public Command
{
public:
	//constructor
	EnableRportCommand(SIPX_INST hInst);
	/*-------------------------------Manipulators----------------------------------*/
	virtual int execute(int argc, char* argv[]);
	/* ============================ ACCESSORS ================================= */
	virtual void getUsage(const char* commandName, UtlString* usage) const;

private:
	SIPX_INST hInst2;
};
#endif //_EnableRportCommand_h_
