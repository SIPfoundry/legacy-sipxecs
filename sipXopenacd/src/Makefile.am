include $(top_srcdir)/config/utility.am

sipxplugin.app: ../ebin/sipxplugin.app
	@$(call SearchAndReplace, $<, $@)

PLUGIN_DIR = @OPENACD_DIR@/plugin.d
EBIN_DIR = @OPENACD_DIR@/lib/ebin

install: sipxplugin.app
	$(INSTALL) -d $(DESTDIR)$(PLUGIN_DIR)
	$(INSTALL) -d $(DESTDIR)$(EBIN_DIR)
	$(INSTALL) ../ebin/*.beam $(DESTDIR)$(EBIN_DIR)
	$(INSTALL) sipxplugin.app $(DESTDIR)$(PLUGIN_DIR)
	$(INSTALL) ../deps/erlmongo/ebin/erlmongo.app $(DESTDIR)$(PLUGIN_DIR)/erlmongo.app

.PHONY: all install sipxplugin.app