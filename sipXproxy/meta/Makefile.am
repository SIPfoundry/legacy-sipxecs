include $(top_srcdir)/config/subdir.am

schemas = \
    forwardingrules.xsd \
    nattraversalrules.xsd

EXTRA_DIST = \
	$(schemas)

install-data-hook : install-schema

.PHONY: install-schema
install-schema: $(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd))

$(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd)): $(DESTDIR)@SIPX_DATADIR@/schema/% : $(srcdir)/%
	$(INSTALL) -D -m 644 $< $@

DISTCLEANFILES =
