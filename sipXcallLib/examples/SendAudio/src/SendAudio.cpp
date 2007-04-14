#include <stdio.h>

#include "mi/CpMediaInterfaceFactoryFactory.h"
#include "mi/CpMediaInterfaceFactory.h"
#include "mi/CpMediaInterface.h"
#include "net/SdpCodecFactory.h"
#include "net/SdpCodec.h"
#include "net/QoS.h"
#include "os/OsTask.h"

int main(int argc, char** argv)
{
   if (argc != 4)
   {
      fprintf(stderr,
              "Usage:  %s local-address remote-address remote-port\n",
              argv[0]);
      exit(1);
   }

   // Address of the targeted recipient.
   const char* local_address = argv[1];

   // SDP describing the remote end.
   const char sdp_text_skeleton[] =
      "v=0\r\n"
      "o=- 1157747613 1157747613 IN IP4 10.1.1.44\r\n"
      "s=Polycom IP Phone\r\n"
      "c=IN IP4 %s\r\n"
      "t=0 0\r\n"
      "a=sendrecv\r\n"
      "m=audio %s RTP/AVP 0 8\r\n"
      "a=rtpmap:0 PCMU/8000\r\n"
      "a=rtpmap:8 PCMA/8000\r\n";
   char sdp_text[sizeof (sdp_text_skeleton) + 20];
   sprintf(sdp_text, sdp_text_skeleton, argv[2], argv[3]);

   // File to play.
   const char* file_name =
      "/home/dworley/sandbox-62/dir-local/var/sipxdata/parkserver/music/default.wav";


   // Create factory for creating CpMediaInterface's.
   CpMediaInterfaceFactory* pMediaInterfaceFactory =
      sipXmediaFactoryFactory(NULL);
   assert(pMediaInterfaceFactory);

   // Construct a codec factory.
   SdpCodecFactory* pCodecFactory = new SdpCodecFactory();
   // Load the supported codecs into it.
   int iRejected;
   pMediaInterfaceFactory->getFactoryImplementation()->
      buildCodecFactory(pCodecFactory, "", "", &iRejected);
   // Get the list of codecs.
   int numCodecs;
   SdpCodec** codecArray;
   pCodecFactory->getCodecs(numCodecs, codecArray);
   assert(numCodecs > 0);

   // Create the CpMediaInterface for the call.
   CpMediaInterface* pMediaInterface =
      pMediaInterfaceFactory->createMediaInterface(local_address,
                                                   local_address,
                                                   numCodecs,
                                                   codecArray,
                                                   // The following are the
                                                   // default values for these
                                                   // parameters in the
                                                   // CpPhoneMediaInterface
                                                   // constructor:
                                                   "",
                                                   QOS_LAYER3_LOW_DELAY_IP_TOS,
                                                   NULL,
                                                   0,
                                                   28);

   assert(pMediaInterface);

   // The connection ID assigned by the CpMediaInterface.
   int connectionId;
   OsStatus s;
   s = pMediaInterface->createConnection(connectionId,
                                         local_address,
                                         NULL);
   assert(s == OS_SUCCESS);

   // Create SdpBody.
   SdpBody sdp(sdp_text);

   // Set the connection remote address parameters.

   int audio_media_line = sdp.findMediaType("audio");
   assert(audio_media_line >= 0);
   UtlString audio_address;
   sdp.getMediaAddress(audio_media_line, &audio_address);
   int audio_port, audio_rtcp_port;
   sdp.getMediaPort(audio_media_line, &audio_port);
   sdp.getMediaRtcpPort(audio_media_line, &audio_rtcp_port);

   s= pMediaInterface->setConnectionDestination(connectionId,
                                                audio_address.data(),
                                                audio_port,
                                                audio_rtcp_port,
                                                PORT_NONE,
                                                PORT_NONE);
   assert(s == OS_SUCCESS);

   // Start RTP transmission.

   // Get the set of payload types.
   int payloads[20], num_payloads;
   sdp.getMediaPayloadType(audio_media_line,
                           sizeof (payloads) / sizeof (payloads[0]),
                           &num_payloads,
                           payloads);
   // Intersect that set with the local codecs.
   SdpCodec *(codecs[20]);
   int num_codecs;
   sdp.getCodecsInCommon(num_payloads,
                         0,
                         payloads,
                         NULL,
                         *pCodecFactory,
                         num_codecs,
                         codecs);
   assert(num_codecs > 0);
   // SipConnection uses an SdpSrtpParameters without initializing it
   // at all, but Mike Cohen says the proper way to initialize it is
   // to memset it to 0's.
   SdpSrtpParameters srtp_parameters;
   memset(&srtp_parameters, 0, sizeof (srtp_parameters));

   s = pMediaInterface->startRtpSend(connectionId,
                                     num_codecs,
                                     codecs,
                                     srtp_parameters);
   assert(s == OS_SUCCESS);

   // Start playing the requested file.

   s = pMediaInterface->playAudio(file_name,
                                  0, // repeat
                                  0, // local
                                  1 // remote
      );
   assert(s == OS_SUCCESS);

   // Wait for a log time.
   OsTask::delay(1000000);

   return 0;
}

#if !defined(_WIN32)
// Dummy definition of JNI_LightButton() to prevent the reference in
// sipXcallLib from producing an error.
void JNI_LightButton(long)
{
}
#endif /* !defined(_WIN32) */
