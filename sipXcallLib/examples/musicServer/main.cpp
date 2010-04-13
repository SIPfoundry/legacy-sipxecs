//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// System includes
#include "os/OsDefs.h"
#include <getopt.h>
#if defined(_WIN32)
#   include <windows.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#endif

// APPLICATION INCLUDES
#include <mp/MpMediaTask.h>
#include <mp/NetInTask.h>
#ifdef INCLUDE_RTCP
#include <rtcp/RTCManager.h>
#endif // INCLUDE_RTCP
#include <net/SipUserAgent.h>
#include <net/SdpCodecFactory.h>
#include <cp/CallManager.h>
#include <ptapi/PtProvider.h>
#include <listener.h>
#include <net/NameValueTokenizer.h>
#include <os/OsConfigDb.h>

// CONSTANTS
#define UDP_PORT                5060    // Default UDP port
#define TCP_PORT                5060    // Default TCP port
#define RTP_START_PORT          9000    // Starting RTP port

#define CODEC_G711_PCMU         "258"   // ID for PCMU
#define CODEC_G711_PCMA         "257"   // ID for PCMA
#define CODEC_DTMF_RFC2833      "128"   // ID for RFC2833 DMTF

#define MAX_CONNECTIONS         9000     // Max number of sim. conns
#define MP_SAMPLE_RATE          8000    // Sample rate (don't change)
#define MP_SAMPLES_PER_FRAME    80      // Frames per second (don't change)

int UdpPort = UDP_PORT;
int TcpPort = TCP_PORT;
int RtpBase = RTP_START_PORT;
enum
{
   Quiet,
   Normal,
   Verbose
} Feedback = Quiet;

const char* LogFile = "musicserver.log";
const char* Playfile = "default.wav";

void initLogger(char* argv[])
{
    OsSysLog::initialize(0, // do not cache any log messages in memory
                         argv[0]); // name for messages from this program
    OsSysLog::setOutputFile(0, // no cache period
                            LogFile); // log file name
    switch (Feedback)
    {
       case Quiet:
          OsSysLog::setLoggingPriority(PRI_WARNING);
          break;
       case Normal:
          OsSysLog::setLoggingPriority(PRI_INFO);
          break;
       case Verbose:
          OsSysLog::setLoggingPriority(PRI_DEBUG);
          break;
    }
    OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);

}

void showHelp(char* argv[])
{
   osPrintf("Usage: \n"
          "   %s:\n"
          "   [ {-u|--udp} <udp-port> ]\n"
          "   [ {-t|--tcp} <tcp-port> ]\n"
          "   [ {-l|--log} <log-file> ]\n"
          "   [ {-v|--verbose} | {-q|--quiet} ]\n"
          "   [ {-h|-?|--help} ]\n"
          "   [ <audio-message-file> ]\n"
          , argv[0]
          );
}

void parseArgs(int argc, char* argv[])
{
   int optResult = 0;

   const char* short_options = "u:t:l:vqh";

   const struct option long_options[] =
      {
         {"verbose", 0, 0, 'v'},
         {"quiet",   0, 0, 'q'},
         {"help",    0, 0, 'h'},
         {"udp",     1, 0, 'u'},
         {"tcp",     1, 0, 't'},
         {"log",     1, 0, 'l'},
         {0, 0, 0, 0}
      };

   while ((optResult = getopt_long (argc, argv, short_options, long_options, NULL)
           ) >= 0)
   {
      char* optend;

      switch (optResult)
      {
         case 'u':
            UdpPort = strtoul(optarg, &optend, 10);
            if ( '\0' != *optend )
            {
               fprintf( stderr, "Invalid UDP port %s\n", optarg );
               exit(1);
            }
            break;

         case 't':
            TcpPort = strtoul(optarg, &optend, 10);
            if ( '\0' != *optend )
            {
               fprintf( stderr, "Invalid TCP port %s\n", optarg );
               exit(1);
            }
            break;

         case 'l':
            LogFile = optarg;
            break;

         case 'v':
            Feedback = Verbose;
            break;

         case 'q':
            Feedback = Quiet;
            break;

         case 'h':
         case '?':
            showHelp(argv);
            exit(0);
            break;

         default:
            fprintf( stderr, "Invalid option %s\n", argv[optind] );
            showHelp(argv);
            exit(1);
            break;
      }
   }

   if (optind < argc)
   {
      Playfile = argv[optind++];
      if ( Feedback != Quiet )
      {
         printf("ready to play '%s'\n", Playfile);
      }
   }

   if (optind < argc)
   {
      fprintf(stderr, "Too many arguments: '%s'\n", argv[optind]);
      showHelp(argv);
      exit(1);
   }
}

int main(int argc, char* argv[])
{
    parseArgs(argc, argv);
    initLogger(argv) ;

    // Bind the SIP user agent to a port and start it up
    SipUserAgent userAgent(TcpPort, UdpPort);
    userAgent.start();

    // Enable PCMU, PCMA, Tones/RFC2833 codecs
    SdpCodecFactory codecFactory;
    SdpCodec::SdpCodecTypes codecs[3];

    codecs[0] = SdpCodecFactory::getCodecType(CODEC_G711_PCMU) ;
    codecs[1] = SdpCodecFactory::getCodecType(CODEC_G711_PCMA) ;
    codecs[2] = SdpCodecFactory::getCodecType(CODEC_DTMF_RFC2833) ;

    codecFactory.buildSdpCodecFactory(3, codecs);

    // Initialize and start up the media subsystem
    OsConfigDb dummyConfigDb;
    mpStartUp(MP_SAMPLE_RATE, MP_SAMPLES_PER_FRAME, 6 * MAX_CONNECTIONS, &dummyConfigDb);
    MpMediaTask::getMediaTask(MAX_CONNECTIONS);
#ifdef INCLUDE_RTCP
    CRTCManager::getRTCPControl();
    osPrintf("RTCP is being used here ...\n");
#endif //INCLUDE_RTCP
    mpStartTasks();

    // Instantiate the call processing subsystem
    UtlString localAddress;
    OsSocket::getHostIp(&localAddress);
    CallManager callManager(FALSE,
                            NULL,
                            TRUE,                              // early media in 180 ringing
                            &codecFactory,
                            RTP_START_PORT,                    // rtp start
                            RTP_START_PORT + (2*MAX_CONNECTIONS), // rtp end
                            localAddress,
                            localAddress,
                            &userAgent,
                            0,                                 // sipSessionReinviteTimer
                            NULL,                              // mgcpStackTask
                            NULL,                              // defaultCallExtension
                            Connection::RING,                  // availableBehavior
                            NULL,                              // unconditionalForwardUrl
                            -1,                                // forwardOnNoAnswerSeconds
                            NULL,                              // forwardOnNoAnswerUrl
                            Connection::BUSY,                  // busyBehavior
                            NULL,                              // sipForwardOnBusyUrl
                            NULL,                              // speedNums
                            CallManager::SIP_CALL,             // phonesetOutgoingCallProtocol
                            4,                                 // numDialPlanDigits
                            CallManager::NEAR_END_HOLD,        // holdType
                            5000,                              // offeringDelay
                            "",                                // pLocal
                            CP_MAXIMUM_RINGING_EXPIRE_SECONDS, // inviteExpiresSeconds
                            QOS_LAYER3_LOW_DELAY_IP_TOS,       // expeditedIpTos
                            MAX_CONNECTIONS,                   // maxCalls
                            NULL);                             // CpMediaInterfaceFactory


    // Create a listener (application) to deal with call
    // processing events (e.g. incoming call and hang ups)
    Listener listener(&callManager,Playfile);
    callManager.addTaoListener(&listener);
    listener.start();

    // Startup the call processing system
    callManager.start();

    // Every minute, dump acknowledge that we are running and dump
    // the call list
    while(1)
    {
        OsTask::delay(60000);
        if ( Verbose == Feedback )
        {
           osPrintf("tick\n");
        }
        if ( Quiet != Feedback )
        {
           callManager.printCalls() ;
        }

        osPrintf("Total number of calls received by musicServer = %d\n", listener.totalCalls());
        osPrintf("Total number of incoming calls received by CallManager = %d\n", callManager.getTotalNumberIncomingCalls());

    }
    return(1);
}


// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
   return 0;
}
