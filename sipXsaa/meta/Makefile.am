include $(top_srcdir)/config/subdir.am

schemas = \
	appearance-groups.xsd

EXTRA_DATA = $(foreach xsd,$(schemas),$(xsd).in)

EXTRA_DIST = \
	$(EXTRA_DATA)

install-data-hook : install-schema

.PHONY: install-schema
install-schema: $(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd))

$(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd)): $(DESTDIR)@SIPX_DATADIR@/schema/% : %
	$(INSTALL) -D -m 644 $< $@

$(foreach xsd,$(schemas),$(xsd)): % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

DISTCLEANFILES = $(schemas)
