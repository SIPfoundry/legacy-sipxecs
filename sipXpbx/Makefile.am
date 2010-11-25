## Process this file with automake to produce Makefile.in
include config/project.am

include config/conditional_doc.am

SUBDIRS = . \
    include \
    src \
    bin \
    etc \
    meta \
    $(doc_SUBDIRS)

EXTRA_DIST = \
    BUILDSTAMP \
    SVN-VERSION \
    CONTRIBUTORS \
    config/apache_version.awk \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipX-config.in \
    config/svn-version \
    config/Doxyfile.in \
    sipxpbx.spec

install-data-hook :
	$(INSTALL) -d $(DESTDIR)@wwwdir@/doc
	$(INSTALL) -d $(DESTDIR)@wwwdir@/conf

dist-hook : rm-cgilib

rm-cgilib:
	rm -rf $(distdir)/src/cgilib $(distdir)/include/cgilib

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
