SUBDIRS = .

AM_CPPFLAGS =  \
	$(SIPXPROXY_COMMON_CXX_FLAGS) \
	-I$(top_srcdir)/include \
	-I/usr/include/libdb4

RESIPROCATE_LIBS = \
	-lcares \
	-ldb_cxx \
	-lrepro \
	-ldum \
	-lresip \
	-lrutil


EXTRA_DIST = \
    config/freeswitch.xml \
    config/monitrc \
    config/pid_from_path.pl \
    config/sipxedge \
    config/sipxedge-setup \
    config/sipxwebrtcd \
    config/freeswitch_edged \
    config/sipxyard_edged


bin_PROGRAMS = sipxcallgen_fs sipxwebrtc sipxedgerpc


sipxcallgen_fs_LDADD = \
	@BOOST_LIBS@ \
	-lpthread \
	-lsipXesl

sipxcallgen_fs_SOURCES =  \
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
	-lpthread \
	$(RESIPROCATE_LIBS) \
	@SIPXCOMMSERVER_LIBS@ \
	-lsipXesl \
	-lmongoclient \
	-ljsonrpccpp

sipxwebrtc_SOURCES =  \
	webrtc/csv_parser.cpp \
	webrtc/csv_parser.hpp \
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

sipxedgerpc_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\"


sipxedgerpc_LDADD = @BOOST_LIBS@ \
	-lpthread \
	@SIPXCOMMSERVER_LIBS@ \
	-lmongoclient \
	-ljsonrpccpp

sipxedgerpc_SOURCES =  \
	rpcserver/RpcServer.h \
	rpcserver/RpcServer.cpp

