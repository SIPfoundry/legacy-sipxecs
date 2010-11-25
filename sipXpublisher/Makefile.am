## Process this file with automake to produce Makefile.in

include config/project.am

include config/conditional_doc.am

SUBDIRS = \
	. \
	src \
	include \
	bin \
	etc \
	$(doc_SUBDIRS)

editscripts_IN = \
    bin/sipstatus.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
     $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
	  $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    config/project.am \
    config/subdir.am \
    sipxpublisher.spec


DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
