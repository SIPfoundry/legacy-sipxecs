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
#include <sipXtapiDriver/CallStartToneCommand.h>

int CallStartToneCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 4)
	{
		if(sipxCallStartTone(atoi(argv[1]), ID_TONE_BUSY,
			atoi(argv[2]), atoi(argv[3])) == SIPX_RESULT_SUCCESS)
		{
			printf("Tone is being played.\n");
		}
		else
		{
			printf("Tone was unable to be played.\n");
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

void CallStartToneCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Call Handle> <1 if should be played locally, else 0>");
	usage->append(" <1 if should be played by remote party, else 0>\n");
}
