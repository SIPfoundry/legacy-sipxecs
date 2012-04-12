include $(top_srcdir)/config/utility.am

sipxplugin.app: ../ebin/sipxplugin.app
	@$(call SearchAndReplace, $<, $@)

PLUGIN_DIR = @OPENACD_DIR@/plugin.d

install: sipxplugin.app
	$(INSTALL) -d $(DESTDIR)$(PLUGIN_DIR)/sipxplugin/ebin
	$(INSTALL) ../ebin/*.beam $(DESTDIR)$(PLUGIN_DIR)/sipxplugin/ebin/
	$(INSTALL) sipxplugin.app $(DESTDIR)$(PLUGIN_DIR)/sipxplugin/ebin/sipxplugin.app
	$(INSTALL) ../deps/erlmongo/ebin/erlmongo.app $(DESTDIR)$(PLUGIN_DIR)/erlmongo.app
	ln -snf $(PLUGIN_DIR)/sipxplugin/ebin/sipxplugin.app $(DESTDIR)$(PLUGIN_DIR)/sipxplugin.app

.PHONY: all install sipxplugin.app