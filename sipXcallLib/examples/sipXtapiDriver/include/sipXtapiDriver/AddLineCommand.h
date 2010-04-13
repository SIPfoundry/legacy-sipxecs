//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _AddLineCommand_h_
#define _AddLineCommand_h_

#include <sipXtapiDriver/CommandProcessor.h>

class AddLineCommand : public Command
{
public:
	//Constructor
	AddLineCommand(const SIPX_INST hInst,
                   SIPX_LINE* phLine);
	/*-------------------------------Manipulators----------------------------------*/
	virtual int execute(int argc, char* argv[]);
	/*-------------------------------Accessors-------------------------------------*/
	virtual void getUsage(const char* commandName, UtlString* usage) const;
private:
	SIPX_INST hInstance;
	SIPX_LINE* phLine2;
};
#endif //_AddLineCommand_h_
