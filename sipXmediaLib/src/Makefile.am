## Process this file with automake to produce Makefile.in

SUBDIRS = test

lib_LTLIBRARIES = libsipXmedia.la


## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXmedia_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age}

if ENABLE_LOCAL_AUDIO
LOCAL_AUDIO_CXXFLAGS = -D_INCLUDE_AUDIO_SUPPORT
endif

libsipXmedia_la_CXXFLAGS = \
	-D_REALTIME_LINUX_AUDIO_THREADS $(LOCAL_AUDIO_CXXFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_builddir)

libsipXmedia_la_LIBADD = \
	@SIPXTACK_LIBS@ @SIPXPORT_LIBS@ @CA_LIBS@ -lstdc++

libsipXmedia_la_SOURCES =  \
    mp/dft.cpp \
    mp/dmaTaskPosix.cpp \
    mp/DSPlib.cpp \
    mp/DspResampling.cpp \
    mp/dtmflib.cpp \
    mp/FilterBank.cpp \
    mp/HandsetFilterBank.cpp \
    mp/mpau.cpp \
    mp/MpAudioAbstract.cpp \
    mp/MpAudioFileDecompress.cpp \
    mp/MpAudioFileOpen.cpp \
    mp/MpAudioFileUtils.cpp \
    mp/MpAudioUtils.cpp \
    mp/MpAudioWaveFileRead.cpp \
    mp/MpBuf.cpp \
    mp/MpBufferMsg.cpp \
    mp/MpCallFlowGraph.cpp \
    mp/MpCodec.cpp \
    mp/MpCodecFactory.cpp \
    mp/MpCodecInfo.cpp \
    mp/MpConnection.cpp \
    mp/MpDecoderBase.cpp \
    mp/MpdGIPSG729ab.cpp \
    mp/MpdGIPSiLBC.cpp \
    mp/MpdGIPSiPCMA.cpp \
    mp/MpdGIPSiPCMU.cpp \
    mp/MpdGIPSiPCMWB.cpp \
    mp/MpdGIPSPCMA.cpp \
    mp/MpdGIPSPCMU.cpp \
    mp/MpdPtAVT.cpp \
    mp/MpdSipxPcma.cpp \
    mp/MpdSipxPcmu.cpp \
    mp/MpeGIPSG729ab.cpp \
    mp/MpeGIPSiLBC.cpp \
    mp/MpeGIPSiPCMA.cpp \
    mp/MpeGIPSiPCMU.cpp \
    mp/MpeGIPSiPCMWB.cpp \
    mp/MpeGIPSPCMA.cpp \
    mp/MpeGIPSPCMU.cpp \
    mp/MpeSipxPcma.cpp \
    mp/MpeSipxPcmu.cpp \
    mp/MpEncoderBase.cpp \
    mp/MpePtAVT.cpp \
    mp/MpFlowGraphBase.cpp \
    mp/MpFlowGraphMsg.cpp \
    mp/mpG711.cpp \
    mp/MpJitterBuffer.cpp \
    mp/MpMediaTask.cpp \
    mp/MpMediaTaskMsg.cpp \
    mp/MpMisc.cpp \
    mp/MpPlayer.cpp \
    mp/MpPlayerEvent.cpp \
    mp/MpRawAudioBuffer.cpp \
    mp/MprBridge.cpp \
    mp/MprDecode.cpp \
    mp/MprDejitter.cpp \
    mp/MprEchoSuppress.cpp \
    mp/MprEncode.cpp \
    mp/MpResource.cpp \
    mp/MpResourceSortAlg.cpp \
    mp/MprFromFile.cpp \
    mp/MprFromMic.cpp \
    mp/MprFromNet.cpp \
    mp/MprFromStream.cpp \
    mp/MprMixer.cpp \
    mp/MprRecorder.cpp \
    mp/MprSplitter.cpp \
    mp/MprToneGen.cpp \
    mp/MprToNet.cpp \
    mp/MprToSpkr.cpp \
    mp/MpStreamFeeder.cpp \
    mp/MpStreamMsg.cpp \
    mp/MpStreamPlayer.cpp \
    mp/MpStreamPlaylistPlayer.cpp \
    mp/MpStreamQueuePlayer.cpp \
    mp/NetInTask.cpp \
    mp/resamplingby2.cpp \
    mp/MpSipxDecoders.cpp \
    mp/StreamBufferDataSource.cpp \
    mp/StreamDataSource.cpp \
    mp/StreamDataSourceListener.cpp \
    mp/StreamDecoderListener.cpp \
    mp/StreamFileDataSource.cpp \
    mp/StreamFormatDecoder.cpp \
    mp/StreamHttpDataSource.cpp \
    mp/StreamQueueingFormatDecoder.cpp \
    mp/StreamQueueMsg.cpp \
    mp/StreamRAWFormatDecoder.cpp \
    mp/StreamWAVFormatDecoder.cpp \
    rtcp/BaseClass.cpp \
    rtcp/ByeReport.cpp \
    rtcp/Message.cpp \
    rtcp/MsgQueue.cpp \
    rtcp/NetworkChannel.cpp \
    rtcp/ReceiverReport.cpp \
    rtcp/RTCManager.cpp \
    rtcp/RTCPConnection.cpp \
    rtcp/RTCPHeader.cpp \
    rtcp/RTCPRender.cpp \
    rtcp/RTCPSession.cpp \
    rtcp/RTCPSource.cpp \
    rtcp/RTCPTimer.cpp \
    rtcp/RTPHeader.cpp \
    rtcp/SenderReport.cpp \
    rtcp/SourceDescription.cpp
