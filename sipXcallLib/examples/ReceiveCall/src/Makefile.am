SUBDIRS =

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = ReceiveCall

ReceiveCall_LDADD = \
	$(top_builddir)/src/libsipXcall.la \
	@SIPXMEDIAADAPTER_LIBS@ \
	@GIPS_NEQ_OBJS@

ReceiveCall_SOURCES = ReceiveCall.cpp
