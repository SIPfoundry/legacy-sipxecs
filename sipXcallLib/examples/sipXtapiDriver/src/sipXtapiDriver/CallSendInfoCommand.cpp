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
#include <sipXtapiDriver/CallSendInfoCommand.h>

CallSendInfoCommand::CallSendInfoCommand(SIPX_INFO* phInfo)
{
	hInfo = phInfo;
}

int CallSendInfoCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 4)
	{
		//char buffer[] = *argv[3];
		if(sipxCallSendInfo(hInfo, atoi(argv[1]), argv[2], argv[3], sizeof(argv[3]))
			== SIPX_RESULT_SUCCESS)
		{
			printf("INFO event sent successfully.\n");
		}
		else
		{
			printf("INFO event unable to be sent.\n");
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

void CallSendInfoCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Call Handle> <INFO content type> <INFO messasge's content>\n");
}
