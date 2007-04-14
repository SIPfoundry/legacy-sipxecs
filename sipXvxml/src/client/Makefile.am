AM_CFLAGS = -DCOMPANY_DOMAIN=L\"pingtel.com\"

AM_CXXFLAGS = -DCOMPANY_DOMAIN=L\"pingtel.com\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\"

noinst_LTLIBRARIES = libOSBclient.la

# do not do versioning for noinst libraries

libOSBclient_la_SOURCES = \
	ivr/IvrUtilTask.cpp \
	ivr/IvrTelListener.cpp \
	api/OSBclient.cpp \
	api/SBclientUtils.cpp \
	api/SBobject.cpp

bin_PROGRAMS = sipXvxml

sipXvxml_SOURCES = \
	$(top_builddir)/config/sipxvxml-buildstamp.h \
	$(top_builddir)/config/sipxvxml-buildstamp.cpp \
	ivr/ConfigFile.cpp \
	ivr/IvrCallListener.cpp \
	ivr/IvrDtmfListener.cpp \
	ivr/clientMain.cpp

sipXvxml_LDADD = \
	libOSBclient.la \
	../inet/libOSBinet.la \
	../jsi/libOSBjsi.la \
	../log/libOSBlog.la \
	../prompt/libOSBprompt.la \
	../rec/libOSBrec.la \
	../tel/libOSBtel.la \
	../trd/libOSBtrd.la \
	../value/libOSBvalue.la \
	../VXI/libVXI.la \
	../../js/libjavascript.la \
	@SIPXCALL_LIBS@ \
	@LIBWWW_LIBS@ \
	@XERCES_LIBS@ \
	@SIPXMEDIAADAPTER_LIBS@ \
	@GIPS_NEQ_OBJS@

noinst_HEADERS = \
	ivr/clientMain.h \
	ivr/ConfigFile.cpp \
	ivr/ConfigFile.h \
	ivr/IvrCallListener.cpp \
	ivr/IvrCallListener.h \
	ivr/IvrDtmfListener.cpp \
	ivr/IvrDtmfListener.h \
	ivr/IvrTelListener.cpp \
	ivr/IvrTelListener.h \
	ivr/IvrUtilTask.cpp \
	ivr/IvrUtilTask.h

# Set the UID bit.  Root permission is required.
install-exec-hook :
	chmod 4755 $(DESTDIR)$(bindir)/sipXvxml
