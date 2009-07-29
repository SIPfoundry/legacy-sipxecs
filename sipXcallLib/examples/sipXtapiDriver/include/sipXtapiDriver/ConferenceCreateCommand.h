//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _ConferenceCreateCommand_h_
#define _ConferenceCreateCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class ConferenceCreateCommand : public Command
{
public:
	//Constructor
	ConferenceCreateCommand(const SIPX_INST hInst,
                            SIPX_CONF*      phConference);
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;
private:
	SIPX_INST hInstance;
	SIPX_CONF* phConf;
};
#endif //_ConferenceCreateCommand_h_
