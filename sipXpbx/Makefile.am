include config/project.am

SUBDIRS = . \
    include \
    src \
    bin \
    etc \
    meta

install-data-hook :
	$(INSTALL) -d $(DESTDIR)@wwwdir@/doc
	$(INSTALL) -d $(DESTDIR)@wwwdir@/conf

@PACKAGE@-dist-hook : rm-cgilib

rm-cgilib:
	rm -rf $(distdir)/src/cgilib $(distdir)/include/cgilib
