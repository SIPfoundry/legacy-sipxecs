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
#include <sipXtapiDriver/ConferenceUnholdCommand.h>

ConferenceUnholdCommand::ConferenceUnholdCommand() {}

int ConferenceUnholdCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 2)
	{
		if(sipxConferenceUnhold(atoi(argv[1])) == SIPX_RESULT_SUCCESS)
		{
			printf("Conference with ID: %d is not on hold.\n", atoi(argv[1]));
		}
		else
		{
			printf("Conference with ID: %d unhold failed.\n",
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

void ConferenceUnholdCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Conference Handle ID>\n");
}
