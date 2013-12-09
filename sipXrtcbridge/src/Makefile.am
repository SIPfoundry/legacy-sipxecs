## Process this file with automake to produce Makefile.in

SUBDIRS = .

## Process this file with automake to produce Makefile.in

RTCBRIDGE_COMMON_CXX_FLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/libjsonrpccpp/src \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

RTCBRIDGE_COMMON_LIBS = \
	@RESIP_LIBS@ \
	@FREESWITCH_LIBS@ \
	@SIPXCOMMSERVER_LIBS@ \
	-lzmq \
	-lmongoclient \
	-lcurl \
	-loss_core \
	-loss_carp \
	-lboost_date_time-mt \
	-lboost_filesystem-mt \
	-lboost_system-mt \
	-lboost_thread-mt \
	-lboost_program_options-mt \
	-lboost_iostreams-mt \
	-lboost_regex-mt \
	-lboost_random-mt \
	-lgtest \
	-lresolv \
	-lcrypt \
	-lrt \
	-ldl \
	-lpthread \
	-lcrypto \
	-lssl \
	-lmcrypt \
	-lltdl \
	-lv8 \
	-lconfig++ \
	-lhiredis \
	-lPocoXML \
	-lPocoNet \
	-lPocoUtil \
	-lpcap
	
bin_PROGRAMS = sipXrtcbridge sipXrtcrpc

sipXrtcbridge_CXXFLAGS = \
	$(RTCBRIDGE_COMMON_CXX_FLAGS)

sipXrtcbridge_LDFLAGS = \
    -shared \
    -rdynamic

sipXrtcbridge_LDADD = \
	libsipXrtcbridge.la \
	$(RTCBRIDGE_COMMON_LIBS)
	

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
    -rdynamic \
    -lcurl

sipXrtcrpc_LDADD = \
	libsipXrtcbridge.la \
	$(RTCBRIDGE_COMMON_LIBS)

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
	ReproGlue.cpp \
	libjsonrpccpp/src/jsonrpc/json/json_internalarray.inl \
	libjsonrpccpp/src/jsonrpc/json/json_valueiterator.inl \
	libjsonrpccpp/src/jsonrpc/json/json_internalmap.inl \
	libjsonrpccpp/src/jsonrpc/json/json_value.cpp \
	libjsonrpccpp/src/jsonrpc/json/json_writer.cpp \
	libjsonrpccpp/src/jsonrpc/json/json_reader.cpp \
	libjsonrpccpp/src/jsonrpc/connectors/httpserver.cpp \
	libjsonrpccpp/src/jsonrpc/connectors/httpclient.cpp \
	libjsonrpccpp/src/jsonrpc/connectors/mongoose.c \
	libjsonrpccpp/src/jsonrpc/errors.cpp \
	libjsonrpccpp/src/jsonrpc/rpcprotocolclient.cpp \
	libjsonrpccpp/src/jsonrpc/specificationwriter.cpp \
	libjsonrpccpp/src/jsonrpc/server.cpp \
	libjsonrpccpp/src/jsonrpc/exception.cpp \
	libjsonrpccpp/src/jsonrpc/procedure.cpp \
	libjsonrpccpp/src/jsonrpc/rpcprotocolserver.cpp \
	libjsonrpccpp/src/jsonrpc/client.cpp \
	libjsonrpccpp/src/jsonrpc/clientconnector.cpp \
	libjsonrpccpp/src/jsonrpc/serverconnector.cpp \
	libjsonrpccpp/src/jsonrpc/specificationparser.cpp

libsipXrtcbridge_la_CXXFLAGS = \
	$(RTCBRIDGE_COMMON_CXX_FLAGS)
	

EXTRA_DIST = \
	freeswitch.xml \
	doublequoteme \
	freeswitch_modules.conf.in \
	freeswitch \
	libjsonrpccpp

BUILT_SOURCES = \
    build-freeswitch \
    freeswitch.xml.inl

dist-hook:
	rm -rf $(distdir)/freeswitch/.git

build-freeswitch:
	make -C freeswitch; make -C freeswitch install  
	
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

