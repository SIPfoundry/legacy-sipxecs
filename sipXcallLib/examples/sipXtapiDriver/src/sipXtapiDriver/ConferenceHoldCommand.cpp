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
#include <sipXtapiDriver/ConferenceHoldCommand.h>

ConferenceHoldCommand::ConferenceHoldCommand() {}

int ConferenceHoldCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 3)
	{
		if(sipxConferenceHold(atoi(argv[1]), atoi(argv[2])) == SIPX_RESULT_SUCCESS)
		{
			printf("Conference with ID: %d on hold.\n", atoi(argv[1]));
		}
		else
		{
			printf("Conference with ID: %d was unable to be put on hold.\n",
				atoi(argv[1]));
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

void ConferenceHoldCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Conference Handle ID> <1(1 = true) for a bridging conference hold, 0(0 = false) for a non-bridging conference hold.>\n");
}
