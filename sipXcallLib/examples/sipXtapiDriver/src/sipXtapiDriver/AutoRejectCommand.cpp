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
#include <sipXtapiDriver/AutoRejectCommand.h>
#include <os/OsDefs.h>
#include <net/SipUserAgent.h>

AutoRejectCommand::AutoRejectCommand(SIPX_INST hInst, SIPX_CALL* hCall, BOOL* isDestroyed)
{
	hInstance = hInst;
	callHandle = hCall;
	destroy = isDestroyed;
}

void AutoRejectCallbackProc(SIPX_CALL hCall,
                            SIPX_LINE hLine,
							SIPX_CALLSTATE_MAJOR eMajor,
							SIPX_CALLSTATE_MINOR eMinor,
							void* pUser)
{
    char szBuffer[128] ;
    char* szEventDesc = sipxCallEventToString(eMajor, eMinor, szBuffer, sizeof(szBuffer)) ;
	if(eMinor == OFFERING_ACTIVE)
	{
		if(sipxCallReject(hCall) == SIPX_RESULT_SUCCESS)
		{
			printf("Call with ID %d has been rejected.\n", hCall);
		}
		else
		{
			printf("Call with ID %d failed to be rejected.\n", hCall);
		}
		sipxCallDestroy(hCall);
	}


}

int AutoRejectCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 1)
	{
		sipxListenerAdd(hInstance, AutoRejectCallbackProc, NULL);
		while(*destroy == FALSE) {} //while call is still connected
		sipxListenerRemove(hInstance, AutoRejectCallbackProc, NULL);

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

void AutoRejectCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" no parameters necessary\n");
}
