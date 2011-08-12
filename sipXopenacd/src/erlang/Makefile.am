source_files = \
	sipxplugin_poller.erl \
	sipxplugin_supervisor.erl \
	sipxplugin_app.erl

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
	test -d $(DESTDIR)$(PLUGIN_DIR) || mkdir -p $(DESTDIR)$(PLUGIN_DIR)
	cd $(DESTDIR)$(PLUGIN_DIR); ln -snf ./sipxplugin/ebin/sipxplugin.app sipxplugin.app 

%.beam : %.erl
	$(ERLC) $(OPENACD_ERLCFLAGS) $(ERL_FLAGS) $<

install-exec-hook:
	cd $(DESTDIR)$(OPENACD_DIR)/plugin.d && ln -s sipxplugin/ebin/sipxplugin.app
