//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _AutoAcceptCommand_h_
#define _AutoAcceptCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class AutoAcceptCommand : public Command
{
public:
	//constructor
	AutoAcceptCommand(SIPX_INST hInst, SIPX_CALL* hCall, BOOL* isDestroyed);
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;

private:
	SIPX_INST hInstance;
	SIPX_CALL* callHandle;
	BOOL* destroy;
};
#endif //_AutoAcceptCommand_h_
