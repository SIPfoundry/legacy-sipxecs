include $(top_srcdir)/config/subdir.am

schemas = \
	authrules.xsd \
	urlmap.xsd

processed_schemas = \
	credential.xsd \
	registration.xsd \
	subscription.xsd

EXTRA_DATA = \
	$(schemas) \
	$(foreach xsd,$(processed_schemas),$(xsd).in)

EXTRA_DIST = \
       $(EXTRA_DATA)

install-data-hook : install-schema

.PHONY: install-schema
install-schema: $(foreach xsd,$(schemas) $(processed_schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd))

# Unprocessed schemas will be in $(srcdir).
$(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd)): $(DESTDIR)@SIPX_DATADIR@/schema/% : $(srcdir)/%
	$(INSTALL) -D -m 644 $< $@

# Processed schemas will be in ".".
$(foreach xsd,$(processed_schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd)): $(DESTDIR)@SIPX_DATADIR@/schema/% : %
	$(INSTALL) -D -m 644 $< $@

$(foreach xsd,$(processed_schemas),$(xsd)): % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
