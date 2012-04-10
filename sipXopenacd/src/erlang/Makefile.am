source_files = \
	sipxplugin.erl \
	sipxplugin_sup.erl \
	sipxplugin_app.erl \
	spx_autoloader.erl \
	spx_log_loader.erl \
	spx_agentconfig_loader.erl \
	spx_freeswitchmedia_loader.erl \
	spx_integration.erl \
	spx_util.erl

compiled_files = $(source_files:.erl=.beam)

EXTRA_DIST = $(source_files) sipxplugin.app.in

PLUGIN_DIR = @OPENACD_DIR@/plugin.d

sipxpluginbindir = $(PLUGIN_DIR)/sipxplugin/ebin
sipxpluginbin_DATA = $(compiled_files) sipxplugin.app
CLEANFILES = $(compiled_files)

sipxplugin.app : sipxplugin.app.in
	sed \
	  -e 's|@PACKAGE\@|@PACKAGE@|g' \
	  -e 's|@VERSION\@|@VERSION@|g' \
	  -e 's|@PACKAGE_REVISION\@|@PACKAGE_REVISION@|g' \
	$< > $@

install-exec-hook :
	$(INSTALL) -d 644 $(DESTDIR)$(PLUGIN_DIR)
	cd $(DESTDIR)$(PLUGIN_DIR) && ln -snf ./sipxplugin/ebin/sipxplugin.app sipxplugin.app 

%.beam : %.erl
	$(ERLC) $(OPENACD_ERLCFLAGS) $(ERL_FLAGS) $<
