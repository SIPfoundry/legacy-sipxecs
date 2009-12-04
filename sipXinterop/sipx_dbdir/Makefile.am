## Process this file with automake to produce Makefile.in
include ../config/subdir.am

include ../interop.am

DBS = \
	caller-alias.xml.in \
	userforward.xml.in \
	userlocation.xml.in

EXTRA_DIST = $(DBS)

install-data-hook: $(foreach db,$(DBS),@SIPX_INTEROP@/db/$(db))

$(foreach db,$(DBS),@SIPX_INTEROP@/db/$(db)) : @SIPX_INTEROP@/db/% : % @SIPX_INTEROP@/db
	$(INSTALL) $< $@

@SIPX_INTEROP@/db :
	$(INSTALL) -d $@
