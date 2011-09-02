SUBDIRS = .

lib_LTLIBRARIES = libsipXzmq.la

INCLUDES = -I$(top_srcdir)/include

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=2
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXzmq_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age} \
	@SSL_LDFLAGS@ \
	@BOOST_LDFLAGS@

libsipXzmq_la_LIBADD = \
	@SSL_LIBS@ \
	-lpthread  \
	-lstdc++ \
	-lboost_system-mt \
	-lboost_thread-mt \
	-lboost_filesystem-mt \
	-lzmq

# this is helpful if you're debugging something with lots of inlines (like the UtlLink code)
# 		-fno-gcse -O0 -fno-implement-inlines -fno-default-inline
libsipXzmq_la_CXXFLAGS = \
	@SSL_CXXFLAGS@ \
	-DHAVE_MEMMOVE=1 \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-fno-gcse -O0 -fno-implement-inlines -fno-default-inline

#
# for now we will be inlining POCO C++ library because it is not
# being packaged yet in CENTOS
#
libsipXzmq_la_SOURCES =  \
	ZMQMessage.cpp \
	ZMQSocket.cpp \
	ZMQSubscriber.cpp \
	ZMQNotifier.cpp \
	ZMQActor.cpp \
	ZMQServiceBroker.cpp \
	ZMQContext.cpp \
	ZMQReactor.cpp \
	ZMQReactorClient.cpp \
	ZMQLogger.cpp \
	ZMQProactorClient.cpp \
	ZMQBlockingRequest.cpp \
	ZMQProactor.cpp \
	ZMQPipeLineBroker.cpp \
	ZMQJsonMessage.cpp \
	ZMQJsonRpcClient.cpp \
	ZMQJsonRpcServer.cpp


DISTCLEANFILES = Makefile.in
