SUBDIRS =

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = PlaceCall

PlaceCall_LDADD = \
	$(top_builddir)/src/libsipXcall.la \
	@SIPXMEDIAADAPTER_LIBS@ \
	@GIPS_NEQ_OBJS@

PlaceCall_SOURCES = PlaceCall.cpp
