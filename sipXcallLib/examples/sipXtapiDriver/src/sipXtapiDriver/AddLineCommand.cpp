//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapi.h>
#include <sipXtapiDriver/AddLineCommand.h>

AddLineCommand::AddLineCommand(const SIPX_INST hInst,
							   SIPX_LINE* phLine)
{
	hInstance = hInst;
	phLine2 = phLine;
}

int AddLineCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 2)
	{
		sipxLineAdd(hInstance, argv[1], phLine2);
		printf("Line Added with ID: %d\n", *phLine2);
	}
	else
	{
		UtlString usage;
        getUsage(argv[0], &usage);
        printf("%s", usage.data());
        commandStatus = CommandProcessor::COMMAND_BAD_SYNTAX;
	}
	return commandStatus;
}

void AddLineCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <The address of record for the line identity>\n");
}
