## Process this file with automake to produce Makefile.in
include config/project.am

include config/conditional_doc.am

SUBDIRS = . \
    include \
    src \
    bin \
    etc \
    meta \
    sipXdialog \
    sipXpage \
    sipXpark \
    sipXpresence \
    sipXrls \
    $(doc_SUBDIRS)

RPMS = $(DEFAULT_RPMS) $(DEBUG_RPM)

EXTRA_DIST = \
    BUILDSTAMP \
    SVN-VERSION \
    CONTRIBUTORS \
    bin/httpd-sipxchange-config.sh.in \
    config/apache_version.awk \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipX-config.in \
    config/svn-version \
    config/Doxyfile.in \
    sipxpbx.spec 

install-exec-hook :
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/bin/httpd-sipxchange-config.sh.in > httpd-sipxchange-config.sh
	$(INSTALL) -D -m 755 httpd-sipxchange-config.sh $(DESTDIR)$(bindir)/httpd-sipxchange-config.sh

install-data-hook :
	$(INSTALL) -d $(DESTDIR)@wwwdir@/doc
	$(INSTALL) -d $(DESTDIR)@wwwdir@/conf

dist-hook : rm-cgilib

rm-cgilib:
	rm -rf $(distdir)/src/cgilib $(distdir)/include/cgilib

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
