## Process this file with automake to produce Makefile.in
include ../config/subdir.am

include ../interop.am

DBS = \
	caller-alias.xml.in \
	userforward.xml.in \
	userlocation.xml.in

EXTRA_DIST = $(DBS)

install-data-hook: $(foreach db,$(DBS),$(DESTDIR)@SIPX_INTEROP@/db/$(db))

$(foreach db,$(DBS),$(DESTDIR)@SIPX_INTEROP@/db/$(db)) : $(DESTDIR)@SIPX_INTEROP@/db/% : % $(DESTDIR)@SIPX_INTEROP@/db
	$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/db : $(DESTDIR)@SIPX_INTEROP@
	$(INSTALL) -d $@

$(DESTDIR)@SIPX_INTEROP@ :
	$(INSTALL) -d $@
