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
#include <tapi/sipXtapiEvents.h>
#include <sipXtapiDriver/AutoAnswerCommand.h>
#include <os/OsDefs.h>
#include <net/SipUserAgent.h>

AutoAnswerCommand::AutoAnswerCommand(SIPX_INST hInst, SIPX_CALL* hCall, BOOL* isDestroyed)
{
	hInstance = hInst;
	callHandle = hCall;
	destroy = isDestroyed;
}

void AutoAnswerCallbackProc(SIPX_CALL hCall,
                            SIPX_LINE hLine,
							SIPX_CALLSTATE_MAJOR eMajor,
							SIPX_CALLSTATE_MINOR eMinor,
							void* pUser)
{
    char szBuffer[128] ;
    char* szEventDesc = sipxCallEventToString(eMajor, eMinor, szBuffer, sizeof(szBuffer)) ;
	if(eMinor == OFFERING_ACTIVE)
	{
		if(sipxCallAccept(hCall) == SIPX_RESULT_SUCCESS)
		{
			printf("Call with ID %d has been accepted.\n", hCall);
		}
		else
		{
			printf("Call with ID %d failed to be accepted.\n", hCall);
		}
		sipxCallAnswer(hCall);
		printf("Call with ID %d has been answered.\n", hCall);
	}
	else if(eMinor == DISCONNECTED_NORMAL)
	{
		sipxCallDestroy(hCall);
	}


}

int AutoAnswerCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 1)
	{
		sipxListenerAdd(hInstance, AutoAnswerCallbackProc, NULL);
		while(*destroy == FALSE)
		{
			OsTask::delay(1500);
		} //while call is still connected
	}
	else
	{
		UtlString usage;
        getUsage(argv[0], &usage);
        printf("%s", usage.data());
        commandStatus = CommandProcessor::COMMAND_BAD_SYNTAX;
	}
	sipxListenerRemove(hInstance, AutoAnswerCallbackProc, NULL);
	return commandStatus;
}

void AutoAnswerCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" no parameters necessary\n");
}
