SUBDIRS =

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = SendAudio

SendAudio_LDADD = \
	$(top_builddir)/src/libsipXcall.la \
	@SIPXMEDIAADAPTER_LIBS@ \
	@GIPS_NEQ_OBJS@

SendAudio_SOURCES = SendAudio.cpp
