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
#include <sipXtapiDriver/ConferenceAddCommand.h>

ConferenceAddCommand::ConferenceAddCommand(SIPX_CALL* hCall)
{
	callHandle = hCall;
}

int ConferenceAddCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 4)
	{
		if(sipxConferenceAdd(atoi(argv[1]), atoi(argv[2]),
			argv[3], callHandle) == SIPX_RESULT_SUCCESS)
		{
			printf("Conference with ID: %d, call ID: %d, line ID: %d.\n",
				atoi(argv[1]), *callHandle, atoi(argv[2]));
		}
		else
		{
			printf("Conference was not added.\n");
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

void ConferenceAddCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Conference Handle ID> <Line Handle ID> <url of the conference partipant to add>\n");
}
