
# preserve header timestamps to preserve incremental builds across
# projects
nobase_includeHEADERS_INSTALL = $(INSTALL) -D -p -c -m 644

#
# for now we will be inlining POCO C++ library because it is not
# being packaged yet in CENTOS
#
nobase_include_HEADERS = \
	ZMQMessage.h \
	ZMQSocket.h \
	ZMQSubscriber.h \
	ZMQNotifier.h \
	ZMQActor.h \
	ZMQServiceBroker.h \
	ZMQContext.h \
	ZMQReactor.h \
	ZMQReactorClient.h \
	ZMQLogger.h \
	ZMQProactorClient.h \
	ZMQBlockingRequest.h \
	ZMQProactor.h \
	ZMQPipeLineBroker.h \
	ZMQJsonMessage.h \
	ZMQJsonRpcClient.h \
	ZMQJsonRpcServer.h
	
DISTCLEANFILES = Makefile.in
