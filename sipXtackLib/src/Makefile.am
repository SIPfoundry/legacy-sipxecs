## Process this file with automake to produce Makefile.in

SUBDIRS = . test

## Make these part of optional compile under IF we use
## GNU tools on windows
EXTRA_DIST = \
    resparse/wnt/inet_addr.c \
    resparse/wnt/writev.c

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_builddir)/include

lib_LTLIBRARIES = libsipXtack.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=2
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXtack_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age} \
	@PCRE_LDFLAGS@ \
	@SSL_LDFLAGS@

libsipXtack_la_CXXFLAGS = \
	@PCRE_CXXFLAGS@ \
	@SSL_CXXFLAGS@ \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DRES_PARSE_NAPTR

# resparse uses .c files, so we need to define the CFLAGS also.
libsipXtack_la_CFLAGS = \
	@PCRE_CFLAGS@ \
	@SSL_CFLAGS@ \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DRES_PARSE_NAPTR

libsipXtack_la_LIBADD = \
	@SIPXPORT_LIBS@ \
	@SSL_LIBS@ \
	-lstdc++

libsipXtack_la_SOURCES =  \
    $(top_builddir)/config/sipxtacklib-buildstamp.h \
    $(top_builddir)/config/sipxtacklib-buildstamp.cpp \
    net/BranchId.cpp \
    net/CallId.cpp \
    net/HttpBody.cpp \
    net/HttpConnection.cpp \
    net/HttpConnectionMap.cpp \
    net/HttpMessage.cpp \
    net/HttpRequestContext.cpp \
    net/HttpServer.cpp \
    net/HttpService.cpp \
    net/MailAttachment.cpp \
    net/MailMessage.cpp \
    net/MimeBodyPart.cpp \
    net/NameValuePair.cpp \
    net/NameValuePairInsensitive.cpp \
    net/NameValueTokenizer.cpp \
    net/NetAttributeTokenizer.cpp \
    net/NetBase64Codec.cpp \
    net/NetMd5Codec.cpp \
    net/ProvisioningAgent.cpp \
    net/ProvisioningAgentXmlRpcAdapter.cpp \
    net/ProvisioningAttrList.cpp \
    net/ProvisioningClass.cpp \
    net/SdpBody.cpp \
    net/SdpCodec.cpp \
    net/SdpCodecFactory.cpp \
    net/SignedUrl.cpp \
    net/SipClient.cpp \
    net/SipClientTcp.cpp \
    net/SipClientTls.cpp \
    net/SipClientUdp.cpp \
    net/SipClientWriteBuffer.cpp \
    net/SipContactDb.cpp \
    net/SipDialog.cpp \
    net/SipDialogEvent.cpp \
    net/SipDialogMgr.cpp \
    net/SipDialogMonitor.cpp \
    net/SipLine.cpp \
    net/SipLineCredentials.cpp \
    net/SipLineEvent.cpp \
    net/SipLineList.cpp \
    net/SipLineMgr.cpp \
    net/SipMessage.cpp \
    net/SipMessageEvent.cpp \
    net/SipMessageList.cpp \
    net/SipNotifyStateTask.cpp \
    net/SipObserverCriteria.cpp \
    net/SipOutputProcessor.cpp \
    net/SipPimClient.cpp \
    net/SipPresenceEvent.cpp \
    net/SipProtocolServerBase.cpp \
    net/SipPublishContentMgr.cpp \
    net/SipPublishServer.cpp \
    net/SipPublishServerEventStateCompositor.cpp \
    net/SipPublishServerEventStateMgr.cpp \
    net/SipRefreshMgr.cpp \
    net/SipRefreshManager.cpp \
    net/SipRequestContext.cpp \
    net/SipResourceList.cpp \
    net/SipServerBroker.cpp \
    net/SipSession.cpp \
    net/SipSrvLookup.cpp \
    net/SipSubscribeClient.cpp \
    net/SipSubscribeServer.cpp \
    net/SipSubscribeServerEventHandler.cpp \
    net/SipSubscriptionMgr.cpp \
    net/SipTcpServer.cpp \
    net/SipTlsServer.cpp \
    net/SipTransaction.cpp \
    net/SipTransactionList.cpp \
    net/SipUdpServer.cpp \
    net/SipUserAgentBase.cpp \
    net/SipUserAgent.cpp \
    net/SipUserAgentStateless.cpp \
    net/SmimeBody.cpp \
    net/StateChangeNotifier.cpp \
    net/TapiMgr.cpp \
    net/Url.cpp \
    net/SipXauthIdentity.cpp \
    net/XmlRpcBody.cpp \
    net/XmlRpcDispatch.cpp \
    net/XmlRpcMethod.cpp \
    net/XmlRpcRequest.cpp \
    net/XmlRpcResponse.cpp \
    resparse/bzero.c \
    resparse/memset.c \
    resparse/ns_name.c \
    resparse/ns_netint.c \
    resparse/res_comp.c  \
    resparse/res_copy.c \
    resparse/res_data.c \
    resparse/res_free.c \
    resparse/res_info.c \
    resparse/res_init.c \
    resparse/res_mkquery.c \
    resparse/res_naptr.c \
    resparse/res_parse.c \
    resparse/res_print.c \
    resparse/res_query.c \
    resparse/res_send.c
