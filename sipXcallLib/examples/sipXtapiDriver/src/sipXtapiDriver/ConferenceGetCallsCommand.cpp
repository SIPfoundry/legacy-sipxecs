//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapi.h>
#include <sipXtapiDriver/ConferenceGetCallsCommand.h>

int ConferenceGetCallsCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 2)
	{
		SIPX_CALL calls[CONF_MAX_CONNECTIONS];
		size_t numCalls;
		if(sipxConferenceGetCalls(atoi(argv[1]), calls, CONF_MAX_CONNECTIONS, numCalls) ==
			SIPX_RESULT_SUCCESS)
		{
			printf("The %d call handles in the conference with id: %d are:\n", numCalls, atoi(argv[1]));
			for(unsigned int i = 0; i < numCalls; i++)
			{
				printf("%d ", calls[i]);
			}
			printf("\n");
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

void ConferenceGetCallsCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Conference handle ID>\n");
}
