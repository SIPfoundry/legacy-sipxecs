## Process this file with automake to produce Makefile.in
include config/project.am

RPMS = $(DEFAULT_RPMS) $(DEBUG_RPM)

include config/conditional_doc.am

SUBDIRS = \
	. \
	js \
	src \
	bin \
	include \
	etc \
	$(doc_SUBDIRS)

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    sipxvxml.spec

install-data-hook :
	$(mkinstalldirs) $(DESTDIR)@wwwdir@/conf

# An alternative to this is to do svn export, but it's very handy to be able
# to create a distribution tarball from a working svn checkout.
dist-hook :
	rm -rf `find $(distdir) -type d -name .svn`

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
