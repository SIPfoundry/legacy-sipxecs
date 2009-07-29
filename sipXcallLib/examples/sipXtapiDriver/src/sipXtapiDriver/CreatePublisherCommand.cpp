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
#include <sipXtapiDriver/CreatePublisherCommand.h>

CreatePublisherCommand::CreatePublisherCommand(const SIPX_INST hInst,
											   SIPX_PUB* phPub)
{
	hInstance = hInst;
	hPub = phPub;
}

int CreatePublisherCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 5)
	{
		if(sipxPublisherCreate(hInstance, hPub, argv[1], argv[2], argv[3], argv[4], sizeof(argv[4]))
			== SIPX_RESULT_SUCCESS)
		{
			printf("Publishing context created with ID: %d\n", *hPub);
		}
		else
		{
			printf("Publishing context was unable to be created.\n");
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

void CreatePublisherCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <resourceId to the state information being published> <type of event that can be published>");
	usage->append(" <the content type being published> <the NOTIFY message's body content>\n");
}
