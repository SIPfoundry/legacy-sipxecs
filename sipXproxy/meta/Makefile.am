include $(top_srcdir)/config/subdir.am

# Pristine copies of sipX 3.0 configuration files.  When no longer
# supporting upgrade from 3.0, you can delete this file and respective
# installation target
SIPX30_CONFIG = @PACKAGE@-3.0.tgz
PREVIOUS_CONFIG = $(DESTDIR)@SIPX_VARDIR@/upgrade/@PACKAGE@-previous.tgz

schemas = \
    forwardingrules.xsd \
    nattraversalrules.xsd 

EXTRA_DIST = \
	$(SIPX30_CONFIG) \
	$(schemas)

install-data-hook : install-schema $(PREVIOUS_CONFIG)

.PHONY: install-schema
install-schema: $(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd))

$(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd)): $(DESTDIR)@SIPX_DATADIR@/schema/% : $(srcdir)/%
	$(INSTALL) -D -m 644 $< $@

$(PREVIOUS_CONFIG) : $(srcdir)/$(SIPX30_CONFIG)
	$(INSTALL) -D -m 644 $(srcdir)/$(SIPX30_CONFIG) $(PREVIOUS_CONFIG)

DISTCLEANFILES = 
