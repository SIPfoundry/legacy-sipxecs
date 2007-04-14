include $(top_srcdir)/config/subdir.am

# Pristine copies of sipX 3.0 configuration files.  When no longer
# supporting upgrade from 3.0, you can delete this file and respective
# installation target
SIPX30_CONFIG = @PACKAGE@-3.0.tgz
PREVIOUS_CONFIG = $(DESTDIR)@SIPX_UPGRADEDIR@/@PACKAGE@-previous.tgz
EXTRA_DIST = \
	$(SIPX30_CONFIG)


install-data-hook : $(PREVIOUS_CONFIG)

$(PREVIOUS_CONFIG) : $(srcdir)/$(SIPX30_CONFIG)
	$(INSTALL) -D -m 644 $(srcdir)/$(SIPX30_CONFIG) $(PREVIOUS_CONFIG)
