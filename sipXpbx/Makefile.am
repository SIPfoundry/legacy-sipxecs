include config/utility.am
include config/project.am

SUBDIRS = . \
    include \
    src \
    bin \
    etc

install-data-hook :
	$(INSTALL) -d $(DESTDIR)@wwwdir@/doc
	$(INSTALL) -d $(DESTDIR)@wwwdir@/conf

dist-hook : rm-cgilib

rm-cgilib:
	rm -rf $(distdir)/src/cgilib $(distdir)/include/cgilib
