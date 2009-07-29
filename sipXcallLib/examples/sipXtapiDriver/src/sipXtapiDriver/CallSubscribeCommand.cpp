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
#include <sipXtapiDriver/CallSubscribeCommand.h>

CallSubscribeCommand::CallSubscribeCommand(SIPX_SUB* hSub)
{
	subHandle = hSub;
}

int CallSubscribeCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 4)
	{
		if(sipxCallSubscribe(atoi(argv[1]), argv[2], argv[3], subHandle)
			== SIPX_RESULT_SUCCESS)
		{
			printf("Subscription successful with call id: %d and subscription id: %d\n",
				 atoi(argv[1]), *subHandle);
		}
		else
		{
			printf("Subscription unsuccessful.\n");
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

void CallSubscribeCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Call Handle> <Type of event that can be published>");
	usage->append(" <the types of NOTIFY events that this client will accept>\n");
}
