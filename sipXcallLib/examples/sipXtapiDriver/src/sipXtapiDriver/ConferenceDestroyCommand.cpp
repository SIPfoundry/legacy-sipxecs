//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
//Application includes
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapi.h>
#include <sipXtapiDriver/ConferenceDestroyCommand.h>

ConferenceDestroyCommand::ConferenceDestroyCommand() {}

int ConferenceDestroyCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 2)
	{
		if(sipxConferenceDestroy(atoi(argv[1])) == SIPX_RESULT_SUCCESS)
		{
			printf("Conference with ID: %d destroyed.\n", atoi(argv[1]));
		}
		else
		{
			printf("Conference with ID: %d was unable to be destroyed.\n", atoi(argv[1]));
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

void ConferenceDestroyCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Conference handle ID>\n");
}
