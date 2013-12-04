## Process this file with automake to produce Makefile.in

SUBDIRS = .

## Process this file with automake to produce Makefile.in

RTCBRIDGE_COMMON_CXX_FLAGS = \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

BUILT_SOURCES = freeswitch.xml.inl

	
freeswitch.xml.inl : freeswitch.xml
	$(top_srcdir)/src/doublequoteme  $(top_srcdir)/src/freeswitch.xml  $(top_srcdir)/src/freeswitch.xml.inl.tmp; \
	mv $(top_srcdir)/src/freeswitch.xml.inl.tmp $(top_srcdir)/src/freeswitch.xml.inl
	echo "#include <string>" > $(top_srcdir)/src/freeswitch_xml.cpp; \
	echo "" >> $(top_srcdir)/src/freeswitch_xml.cpp; \
	echo "" >> $(top_srcdir)/src/freeswitch_xml.cpp; \
	echo "std::string freeswitch_xml(" >> $(top_srcdir)/src/freeswitch_xml.cpp; \
	cat $(top_srcdir)/src/freeswitch.xml.inl >> $(top_srcdir)/src/freeswitch_xml.cpp; \
	echo "); // EOF" >> $(top_srcdir)/src/freeswitch_xml.cpp; \
	touch $(top_srcdir)/src/freeswitch_xml.cpp; \
	rm -f $(top_srcdir)/src/freeswitch.xml.inl

bin_PROGRAMS = sipXrtcbridge sipXrtcrpc

sipXrtcbridge_CXXFLAGS = \
	$(RTCBRIDGE_COMMON_CXX_FLAGS)

sipXrtcbridge_LDFLAGS = \
    -shared \
    -rdynamic

sipXrtcbridge_LDADD = \
	libsipXrtcbridge.la \
	@SIPXCOMMSERVER_LIBS@ \
	@RESIP_LIBS@ \
	@FREESWITCH_LIBS@ \
	@JSONRPC_LIBS@ \
	-lzmq \
	-lmongoclient \
	-lboost_system-mt

sipXrtcbridge_SOURCES =  \
	WSRouterMain.cpp \
	WSRouter.h \
	WSRouter.cpp \
	freeswitch_xml.cpp \
	AuthInformationGrabber.h \
	AuthInformationGrabber.cpp \
	DomainConfig.h \
	DomainConfig.cpp
	
sipXrtcrpc_CXXFLAGS = \
	$(RTCBRIDGE_COMMON_CXX_FLAGS)

sipXrtcrpc_LDFLAGS = \
    -shared \
    -rdynamic

sipXrtcrpc_LDADD = \
	@SIPXCOMMSERVER_LIBS@ \
	@RESIP_LIBS@ \
	@JSONRPC_LIBS@ \
	-lmongoclient

sipXrtcrpc_SOURCES =  \
	rpc/RpcServer.cpp

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

lib_LTLIBRARIES = libsipXrtcbridge.la

libsipXrtcbridge_la_LDFLAGS = \
    -shared \
    -version-info ${version_Current}:${version_Revision}:${version_Age}


libsipXrtcbridge_la_SOURCES = \
    AuthIdentityEncoder.cpp \
    EslConnection.cpp \
    EslEvent.cpp \
    EslListener.cpp \
    FreeSwitchRunner.cpp \
    ReproGlue.cpp

libsipXrtcbridge_la_CXXFLAGS = \
	$(RTCBRIDGE_COMMON_CXX_FLAGS)
	

EXTRA_DIST = \
  freeswitch.xml \
  doublequoteme



        
