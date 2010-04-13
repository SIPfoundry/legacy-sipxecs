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
#include <sipXtapiDriver/ConferenceJoinCommand.h>

ConferenceJoinCommand::ConferenceJoinCommand() {}

int ConferenceJoinCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 3)
	{
		if(sipxConferenceJoin(atoi(argv[1]), atoi(argv[2])) == SIPX_RESULT_SUCCESS)
		{
			printf("Conferenced joined with ID: %d and call ID: %d.\n",
				atoi(argv[1]), atoi(argv[2]));
		}
		else
		{
			printf("Conference was not joined.\n");
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

void ConferenceJoinCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Conference Handle ID> <Call Handle ID>\n");
}
