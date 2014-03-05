SUBDIRS = common/esl . 

AM_CPPFLAGS =  \
	$(SIPXPROXY_COMMON_CXX_FLAGS) \
	-I$(top_srcdir)/include \
	-I${srcdir}/common/esl/include \
	-I${top_srcdir}/libs/resiprocate \
	-I${top_srcdir}/libs/libjson-rpc-cpp/src/jsonrpc \
	-I${top_srcdir}/libs/csv_parser/include


bin_PROGRAMS = sipxcallgen sipxwebrtc

sipxcallgen_CFLAGS = -I${srcdir}/common/esl/include

sipxcallgen_LDADD = @BOOST_LIBS@ -lpthread @SIPX_EDGE_BUILD_DIR@/applications/common/esl/src/.libs/libsipxedge_esl.a

sipxcallgen_SOURCES =  \
	callgen/CallManager.h \
	callgen/CallManager.cpp \
	callgen/CallThread.h \
	callgen/CallThread.cpp \
	callgen/main.cpp

sipxwebrtc_CFLAGS = -I${srcdir}/common/esl/include

sipxwebrtc_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\"
	

sipxwebrtc_LDADD = @BOOST_LIBS@ \
	-lpthread @SIPX_EDGE_BUILD_DIR@/applications/common/esl/src/.libs/libsipxedge_esl.a \
	@RESIPROCATE_LIBS@ \
	@JSON_RPC_LIBS@ \
	@CSV_PARSER_LIBS@ \
	@SIPXCOMMSERVER_LIBS@ \
	-lzmq \
	-lmongoclient

sipxwebrtc_SOURCES =  \
	webrtc/WSRouterMain.cpp \
	webrtc/AuthIdentityEncoder.h \
	webrtc/AuthIdentityEncoder.cpp \
	webrtc/DomainConfig.h \
	webrtc/DomainConfig.cpp \
	webrtc/ReproGlue.h \
	webrtc/ReproGlue.cpp \
	webrtc/AuthInformationGrabber.h\
	webrtc/AuthInformationGrabber.cpp\
	webrtc/WSRouter.h \
	webrtc/WSRouter.cpp



