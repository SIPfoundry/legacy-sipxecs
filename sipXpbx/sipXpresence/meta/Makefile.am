include $(top_srcdir)/config/subdir.am

schemas = \
	presence-state.xsd

EXTRA_DATA = $(foreach xsd,$(schemas),$(xsd).in)

EXTRA_DIST = \
	$(EXTRA_DATA)

install-data-hook : install-schema

.PHONY : install-schema
install-schema: $(foreach xsd,$(schemas),$(DESTDIR)@SIPX_SCHEMADIR@/$(xsd))

$(foreach xsd,$(schemas),$(DESTDIR)@SIPX_SCHEMADIR@/$(xsd)): $(DESTDIR)@SIPX_SCHEMADIR@/% : %
	$(INSTALL) -D -m 644 $< $@

$(foreach xsd,$(schemas),$(xsd)): % : $(srcdir)/%.in
	$(LocalizeSipXconfig) $< > $@

DISTCLEANFILES = \
	$(schemas)
