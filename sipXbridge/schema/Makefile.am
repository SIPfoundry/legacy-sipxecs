
schemas = \
	peeridentities.xsd

EXTRA_DIST = \
	$(schemas)

EXTRA_DATA = \
	$(schemas)

install-data-hook : install-schema

.PHONY: install-schema
install-schema: $(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd))

$(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/%: $(srcdir)/%)
	$(INSTALL) -D -m 644 $< $@
