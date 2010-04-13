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
#include <sipXtapiDriver/CallCreateCommand.h>

CallCreateCommand :: CallCreateCommand(const SIPX_INST hInst,
									   SIPX_CALL* phCall)
{
	hInstance = hInst;
	phCall2 = phCall;
}

int CallCreateCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 2)
	{
		if(sipxCallCreate(hInstance, atoi(argv[1]), phCall2) == SIPX_RESULT_SUCCESS)
		{
			printf("Call created with ID: %d\n", *phCall2);
		}
		else
		{
			printf("Call unable to be created.\n");
		}
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

void CallCreateCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <line Identity for the outbound call>\n");
}
