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
#include <sipXtapiDriver/CallAcceptCommand.h>

CallAcceptCommand::CallAcceptCommand()
{
}

int CallAcceptCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 2)
	{
		if(sipxCallAccept(atoi(argv[1])) == SIPX_RESULT_SUCCESS)
		{
			printf("Call with ID %d has been accepted.\n", atoi(argv[1]));
		}
		else
		{
			printf("Call with ID %d failed to be accepted.\n", atoi(argv[1]));
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

void CallAcceptCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append("PARAM 1: Call Handle\n");
}
