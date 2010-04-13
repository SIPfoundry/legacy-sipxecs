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
#include <sipXtapiDriver/UpdatePublisherCommand.h>

int UpdatePublisherCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 4)
	{
		if(sipxPublisherUpdate(atoi(argv[1]), argv[2], argv[3], sizeof(argv[3]))
			== SIPX_RESULT_SUCCESS)
		{
			printf("Publishing context updated with ID: %d\n", atoi(argv[1]));
		}
		else
		{
			printf("Publishing context was unable to be updated.\n");
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

void UpdatePublisherCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
	usage->append(" <publisher handle> <the content type being published> <the NOTIFY message's body content>\n");
}
