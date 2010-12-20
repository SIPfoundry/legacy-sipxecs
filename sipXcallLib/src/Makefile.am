## Process this file with automake to produce Makefile.in

EXTRA_DIST = tapi

SUBDIRS = . test

INCLUDES = -I$(top_srcdir)/include

lib_LTLIBRARIES = libsipXcall.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXcall_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXcall_la_LIBADD = \
	@SIPXMEDIAADAPTER_LIBS@ \
	@SIPXMEDIA_LIBS@ \
	@SIPXTACK_LIBS@ \
	@SIPXPORT_LIBS@ \
	-lstdc++

libsipXcall_la_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

libsipXcall_la_SOURCES = \
    cp/CallManager.cpp \
    cp/Connection.cpp \
    cp/CpCall.cpp \
    cp/CpCallManager.cpp \
    cp/CpGatewayManager.cpp \
    cp/CpGhostConnection.cpp \
    cp/CpIntMessage.cpp \
    cp/CpMultiStringMessage.cpp \
    cp/CpPeerCall.cpp \
    cp/CpStringMessage.cpp \
    cp/SipConnection.cpp \
    cp/DialogEventPublisher.cpp \
    cp/LinePresenceMonitor.cpp \
    cp/LinePresenceMonitorMsg.cpp \
    cp/PresenceDialInServer.cpp \
    cp/SipPresenceMonitor.cpp \
    cp/XmlRpcSignIn.cpp \
    ptapi/PtAddessCapabilities.cpp \
    ptapi/PtAddress.cpp \
    ptapi/PtAddressForwarding.cpp \
    ptapi/PtAudioCodec.cpp \
    ptapi/PtCall.cpp \
    ptapi/PtCallCapabilities.cpp \
    ptapi/PtCallEvent.cpp \
    ptapi/PtCallListener.cpp \
    ptapi/PtComponent.cpp \
    ptapi/PtComponentGroup.cpp \
    ptapi/PtComponentGroupCapabilities.cpp \
    ptapi/PtConfigDb.cpp \
    ptapi/PtConnection.cpp \
    ptapi/PtConnectionCapabilities.cpp \
    ptapi/PtConnectionEvent.cpp \
    ptapi/PtConnectionListener.cpp \
    ptapi/PtEvent.cpp \
    ptapi/PtEventListener.cpp \
    ptapi/PtMediaCapabilities.cpp \
    ptapi/PtMetaEvent.cpp \
    ptapi/PtMultiCallMetaEvent.cpp \
    ptapi/PtPhoneButton.cpp \
    ptapi/PtPhoneDisplay.cpp \
    ptapi/PtPhoneExtSpeaker.cpp \
    ptapi/PtPhoneGraphicDisplay.cpp \
    ptapi/PtPhoneHookswitch.cpp \
    ptapi/PtPhoneLamp.cpp \
    ptapi/PtPhoneMicrophone.cpp \
    ptapi/PtPhoneRinger.cpp \
    ptapi/PtPhoneSpeaker.cpp \
    ptapi/PtPhoneTerminal.cpp \
    ptapi/PtPhoneTextDisplay.cpp \
    ptapi/PtProvider.cpp \
    ptapi/PtProviderCapabilities.cpp \
    ptapi/PtProviderListener.cpp \
    ptapi/PtSessionDesc.cpp \
    ptapi/PtSingleCallMetaEvent.cpp \
    ptapi/PtStubs.cpp \
    ptapi/PtTerminal.cpp \
    ptapi/PtTerminalCapabilities.cpp \
    ptapi/PtTerminalComponentEvent.cpp \
    ptapi/PtTerminalComponentListener.cpp \
    ptapi/PtTerminalConnection.cpp \
    ptapi/PtTerminalConnectionCapabilities.cpp \
    ptapi/PtTerminalConnectionEvent.cpp \
    ptapi/PtTerminalConnectionListener.cpp \
    ptapi/PtTerminalEvent.cpp \
    ptapi/PtTerminalListener.cpp \
    tao/TaoAdaptor.cpp \
    tao/TaoAddressAdaptor.cpp \
    tao/TaoCallAdaptor.cpp \
    tao/TaoClientTask.cpp \
    tao/TaoConnectionAdaptor.cpp \
    tao/TaoEvent.cpp \
    tao/TaoEventDispatcher.cpp \
    tao/TaoEventListener.cpp \
    tao/TaoListenerClient.cpp \
    tao/TaoListenerEventMessage.cpp \
    tao/TaoListenerManager.cpp \
    tao/TaoListeningTask.cpp \
    tao/TaoMessage.cpp \
    tao/TaoObjectMap.cpp \
    tao/TaoPhoneComponentAdaptor.cpp \
    tao/TaoProviderAdaptor.cpp \
    tao/TaoProviderListener.cpp \
    tao/TaoReference.cpp \
    tao/TaoServerTask.cpp \
    tao/TaoString.cpp \
    tao/TaoTerminalAdaptor.cpp \
    tao/TaoTerminalConnectionAdaptor.cpp \
    tao/TaoTerminalConnectionListener.cpp \
    tao/TaoTransportAgent.cpp \
    tao/TaoTransportTask.cpp \
    ps/PsButtonInfo.cpp \
    ps/PsButtonTask.cpp \
    ps/PsHookswDev.cpp \
    ps/linux/PsHookswDevLinux.cpp \
    ps/PsHookswTask.cpp \
    ps/PsKeybdDev.cpp \
    ps/PsLampDev.cpp \
    ps/linux/PsLampDevLinux.cpp \
    ps/PsLampInfo.cpp \
    ps/PsLampTask.cpp \
    ps/PsMsg.cpp \
    ps/PsPhoneTask.cpp \
    ps/PsTaoButton.cpp \
    ps/PsTaoComponent.cpp \
    ps/PsTaoComponentGroup.cpp \
    ps/PsTaoDisplay.cpp \
    ps/PsTaoHookswitch.cpp \
    ps/PsTaoLamp.cpp \
    ps/PsTaoMicrophone.cpp \
    ps/PsTaoRinger.cpp \
    ps/PsTaoSpeaker.cpp \
    tapi/sipXtapi.cpp \
    tapi/sipXtapiEvents.cpp \
    tapi/sipXtapiInternal.cpp \
    tapi/SipXHandleMap.cpp \
    tapi/SipXMessageObserver.cpp

noinst_HEADERS = \
    cp/ConfirmationTone.h \
    cp/DialTone.h \
    cp/BusyTone.h

DISTCLEANFILES = Makefile.in
