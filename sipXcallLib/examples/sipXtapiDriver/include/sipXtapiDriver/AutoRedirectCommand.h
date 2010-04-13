//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _AutoRedirectCommand_h_
#define _AutoRedirectCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapiEvents.h>

class AutoRedirectCommand : public Command
{
public:
	//Constructor
	AutoRedirectCommand(SIPX_INST hInst, SIPX_CALL hCall, BOOL* isDestroyed);
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;

private:
	SIPX_INST hInstance;
	SIPX_CALL callHandle;
	BOOL* destroy;
};
#endif //_AutoRedirectCommand_h_
