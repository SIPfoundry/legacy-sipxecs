## Process this file with automake to produce Makefile.in

include config/project.am

RPMS = $(DEFAULT_RPMS) $(DEBUG_RPM)
unconditional_SUBDIRS = . src bin meta etc

include config/conditional_doc.am

editscripts_IN = \
    bin/sipxsaa.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
          $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
	  $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

EXTRA_DIST = \
    BUILDSTAMP \
    SVN-VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    sipxsaa.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)

