## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/project.am

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    CONTRIBUTORS \
    SVN-VERSION \
    BUILDSTAMP \
    @PACKAGE@-rpmlintrc \
    sipxmediaadapterlib.spec

include config/conditional_doc.am

SUBDIRS = \
	. \
	interface \
	sipXmediaMediaProcessing \
	sipXmediaVoiceEngine \
	sipXmediaConferenceEngine \
	$(doc_SUBDIRS)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
