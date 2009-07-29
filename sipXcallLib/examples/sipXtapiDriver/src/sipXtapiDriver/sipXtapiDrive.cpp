//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#if defined(_WIN32)
# include <io.h>
# define STDIN_FILENO 0 /* can't find where windows defines this */
#else
# include <unistd.h>
#endif

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <sipXtapiDriver/CommandProcessor.h>
#include <net/SipUserAgent.h>
#include <net/SipLineMgr.h>
#include <net/SipRefreshMgr.h>
#include <sipXtapiDriver/ExitCommand.h>
#include <sipXtapiDriver/HelpCommand.h>
#include <sipXtapiDriver/HistoryCommand.h>
#include <sipXtapiDriver/CommandMsgProcessor.h>
#include <tapi/sipXtapi.h>
#include <tapi/sipXtapiEvents.h>
#include <sipXtapiDriver/SleepCommand.h>
#include <sipXtapiDriver/EnableStunCommand.h>
#include <sipXtapiDriver/EnableRportCommand.h>
#include <sipXtapiDriver/AddLineCommand.h>
#include <sipXtapiDriver/CallCreateCommand.h>
#include <sipXtapiDriver/CallConnectCommand.h>
#include <sipXtapiDriver/CallDestroyCommand.h>
#include <sipXtapiDriver/CallAcceptCommand.h>
#include <sipXtapiDriver/CallAnswerCommand.h>
#include <sipXtapiDriver/CallHoldCommand.h>
#include <sipXtapiDriver/CallRedirectCommand.h>
#include <sipXtapiDriver/ConferenceCreateCommand.h>
#include <sipXtapiDriver/ConferenceDestroyCommand.h>
#include <sipXtapiDriver/ConferenceJoinCommand.h>
#include <sipXtapiDriver/ConferenceAddCommand.h>
#include <sipXtapiDriver/ConferenceHoldCommand.h>
#include <sipXtapiDriver/ConferenceUnholdCommand.h>
#include <sipXtapiDriver/CallPlayFileCommand.h>
#include <sipXtapiDriver/CallStartToneCommand.h>
#include <sipXtapiDriver/CallSendInfoCommand.h>
#include <sipXtapiDriver/CallSubscribeCommand.h>
#include <sipXtapiDriver/CreatePublisherCommand.h>
#include <sipXtapiDriver/AutoAnswerCommand.h>
#include <sipXtapiDriver/AutoRejectCommand.h>
#include <sipXtapiDriver/AutoRedirectCommand.h>
#include <sipXtapiDriver/UpdatePublisherCommand.h>
#include <sipXtapiDriver/DestroyPublisherCommand.h>
#include <sipXtapiDriver/CallUnholdCommand.h>
#include <sipXtapiDriver/ConferenceGetCallsCommand.h>
#include <sipXtapiDriver/AutoAcceptCommand.h>
#include <sipXtapiDriver/KeyCommand.h>

#define MAX_RECORD_EVENTS       16

SIPX_INST g_hInst;    //global variable of the instance pointer from sipXInitialize
SIPX_CALL g_hCall;
SIPX_CALLSTATE_MAJOR    g_eRecordEvents[MAX_RECORD_EVENTS] ;    // List of last N events
int                     g_iNextEvent ;      // Index for g_eRecordEvents ringer buffer
BOOL isDestroyed = FALSE; //needed for auto commands

// Print usage message
void usage(const char* szExecutable)
{
    char szBuffer[64];

    sipxConfigGetVersion(szBuffer, 64);
    printf("\nUsage:\n") ;
    printf("   %s <options> [URL]\n", szExecutable) ;
    printf("      using %s\n", szBuffer);
    printf("\n") ;
    printf("Options:\n") ;
    printf("   -p SIP port (default = 5060)\n") ;
    printf("   -r RTP port start (default = 9000)\n") ;
	printf("   -t TCP port (default = %d) \n", DEFAULT_TCP_PORT);
    printf("   -b bind to address (default 0.0.0.0)\n") ;
    printf("\n") ;
}

// Parse arguments
bool parseArgs(int argc,
               char*  argv[],
               int*   pSipPort,
               int*   pRtpPort,
			   int*   pTcpPort,
               char** bindToAddr)
{
    bool bRC = true ;
    *pSipPort = 5060 ;
    *pRtpPort = 9000 ;
	*pTcpPort = DEFAULT_TCP_PORT;
    *bindToAddr = "0.0.0.0";

    for (int i=1; i<argc; i++)
    {
       if (strcmp(argv[i], "-p") == 0)
	   {
		   if ((i+1) < argc)
		   {
			   *pSipPort = atoi(argv[++i]) ;
           }
           else
		   {
			   bRC = false;
               break ; // Error
           }
       }
       else if (strcmp(argv[i], "-r") == 0)
       {
           if ((i+1) < argc)
           {
               *pRtpPort = atoi(argv[++i]) ;
           }
           else
           {
			   bRC = false;
			   break ; // Error
           }
       }
	   else if(strcmp(argv[i], "-t") == 0)
	   {
		   if((i+1) < argc)
		   {
			   *pRtpPort = atoi(argv[++i]);
		   }
		   else
		   {
			   bRC = false;
			   break;
		   }
	   }
       else if (strcmp(argv[i], "-b") == 0)
       {
           if ((i+1) < argc)
           {
               *bindToAddr = strdup(argv[++i]) ;
           }
           else
           {
			   bRC = false;
               break ; // Error
		   }
       }

    }
    return bRC ;
}

// Event callback -- records last MAX_RECORD_EVENTS events
void EventCallbackProc( SIPX_CALL hCall,
                        SIPX_LINE hLine,
                        SIPX_CALLSTATE_MAJOR eMajor,
                        SIPX_CALLSTATE_MINOR eMinor,
                        void* pUser)
{
    char szBuffer[128] ;
    char* szEventDesc = sipxCallEventToString(eMajor, eMinor, szBuffer, sizeof(szBuffer)) ;
	g_hCall = hCall;
    printf("<-> Received Event: %s CallHandle: %d\n", szEventDesc, hCall) ;
	if(eMinor == DESTROYED_NORMAL)
	{
		g_hCall = 0;
		isDestroyed = TRUE;
	}
    g_eRecordEvents[g_iNextEvent] = eMajor ;
    g_iNextEvent = (g_iNextEvent + 1) % MAX_RECORD_EVENTS ;
}

int main(int argc, char* argv[])
{
	int iSipPort, iRtpPort, iTcpPort;
	char* bindToAddr;
	char* szServer = NULL;
	SIPX_LINE hLine;
	SIPX_CALL hCall;
	SIPX_CONF hConf;
	SIPX_INFO hInfo;
	SIPX_SUB hSub;
	SIPX_PUB hPub;

	UtlBoolean commandStatus = CommandProcessor::COMMAND_SUCCESS;
	char buffer[1024];
	char buffer2[64];
	char* commandLine;
	CommandProcessor processor;

	SipLineMgr*    lineMgr = new SipLineMgr();
	SipRefreshMgr* refreshMgr = new SipRefreshMgr();

	lineMgr->StartLineMgr();
	lineMgr->initializeRefreshMgr( refreshMgr );
	if (parseArgs(argc, argv, &iSipPort, &iRtpPort, &iTcpPort, &bindToAddr))
	{
		sipxInitialize(&g_hInst, iSipPort, iTcpPort, DEFAULT_TLS_PORT, iRtpPort, DEFAULT_CONNECTIONS,
						DEFAULT_IDENTITY, bindToAddr);
		sipxListenerAdd(g_hInst, EventCallbackProc, NULL) ;
	}
	else
	{
		usage(argv[0]);
	}
	processor.fill();
	processor.registerCommand("help", new HelpCommand(&processor));
	processor.registerCommand("?", new HelpCommand(&processor));
	processor.registerCommand("history", new HistoryCommand(&processor));
	processor.registerCommand("sleep", new SleepCommand());
	processor.registerCommand("quit", new ExitCommand());
	processor.registerCommand("exit", new ExitCommand());
	processor.registerCommand("enablestun", new EnableStunCommand(g_hInst));
	processor.registerCommand("enablerport", new EnableRportCommand(g_hInst));
	processor.registerCommand("addline", new AddLineCommand(g_hInst, &hLine));
	processor.registerCommand("callcreate", new CallCreateCommand(g_hInst, &hCall));
	processor.registerCommand("callconnect", new CallConnectCommand());
	processor.registerCommand("calldestroy", new CallDestroyCommand());
	processor.registerCommand("callaccept", new CallAcceptCommand());
	processor.registerCommand("callanswer", new CallAnswerCommand());
	processor.registerCommand("callhold", new CallHoldCommand());
	processor.registerCommand("callredirect", new CallRedirectCommand());
	processor.registerCommand("conferencecreate", new ConferenceCreateCommand(g_hInst, &hConf));
	processor.registerCommand("conferencedestroy", new ConferenceDestroyCommand());
	processor.registerCommand("conferencejoin", new ConferenceJoinCommand());
	processor.registerCommand("conferenceadd", new ConferenceAddCommand(&hCall));
	processor.registerCommand("conferencehold", new ConferenceHoldCommand());
	processor.registerCommand("conferenceunhold", new ConferenceUnholdCommand());
	processor.registerCommand("callplayfile", new CallPlayFileCommand());
	processor.registerCommand("callstarttone", new CallStartToneCommand());
	processor.registerCommand("callsendinfo", new CallSendInfoCommand(&hInfo));
	processor.registerCommand("callsubscribe", new CallSubscribeCommand(&hSub));
	processor.registerCommand("createpublisher", new CreatePublisherCommand(g_hInst, &hPub));
	processor.registerCommand("autoanswer", new AutoAnswerCommand(g_hInst, &g_hCall, &isDestroyed));
	processor.registerCommand("autoreject", new AutoRejectCommand(g_hInst, &hCall, &isDestroyed));
	processor.registerCommand("autoredirect", new AutoRedirectCommand(g_hInst, hCall, &isDestroyed));
	processor.registerCommand("updatepublisher", new UpdatePublisherCommand());
	processor.registerCommand("destroypublisher", new DestroyPublisherCommand());
	processor.registerCommand("callunhold", new CallUnholdCommand());
	processor.registerCommand("conferencegetcalls", new ConferenceGetCallsCommand());
	processor.registerCommand("autoaccept", new AutoAcceptCommand(g_hInst, &g_hCall, &isDestroyed));
	processor.registerCommand("key", new KeyCommand());

	//Initialization
	UtlBoolean doPrompt = isatty(STDIN_FILENO);

	if ( doPrompt )
	{
		sipxConfigGetVersion(buffer2, 64);
		printf("Version: %s\n", buffer2);
		printf("SIP port: %d   RTP port: %d\n", iSipPort, iRtpPort);
		printf("\nEnter command or help/? for help\n");
        sipxLineAdd(g_hInst, "sipxtapidriver@localhost", &hLine) ;
        printf("Added line for \"sipxtapidriver@localhost\" id=%d\n", hLine) ;

		printf("sipXtapi Driver: ");
	}

	for ( commandStatus = CommandProcessor::COMMAND_SUCCESS;
		(   commandStatus < CommandProcessor::COMMAND_FAILED_EXIT
			&& commandStatus != CommandProcessor::COMMAND_SUCCESS_EXIT
			&& (commandLine = fgets(buffer,1024,stdin))
			);
      )
	{
		//printf("GOT command line:\"%s\"\n", commandLine);

		commandStatus = processor.executeCommand(commandLine);
		isDestroyed = FALSE;
		//printf("command status: %d exit status: %d\n", commandStatus,
		//CommandProcessor::COMMAND_SUCCESS_EXIT);
		if ( doPrompt && !((strncmp(commandLine, "exit", 4) == 0) || (strncmp(commandLine, "quit", 4) == 0)) )
		{
            printf("sipXtapi Driver: ");
		}
	}
	sipxListenerRemove(g_hInst, EventCallbackProc, NULL);
	return CommandProcessor::COMMAND_SUCCESS_EXIT != commandStatus;
}
