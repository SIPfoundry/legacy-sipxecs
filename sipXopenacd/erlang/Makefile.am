REBAR=./rebar

openacd_src = $(srcdir)/src
rebar= $(srcdir)/rebar \
	$(srcdir)/rebar.config
openacd_erlang_srcdir = $(abs_builddir)

all: deps compile

deps:
	$(shell cp -r $(openacd_src) $(openacd_erlang_srcdir) )
	$(shell cp -r $(rebar) $(openacd_erlang_srcdir) )
	$(REBAR) get-deps update-deps

compile:
	$(shell cp -r $(openacd_src) $(openacd_erlang_srcdir) )
	$(shell cp -r $(rebar) $(openacd_erlang_srcdir) )
	$(REBAR) compile skip_deps=true

compiled_files = *.beam

PLUGIN_DIR = @OPENACD_DIR@/plugin.d

CLEANFILES = $(compiled_files)

sipxplugin.app :
	sed \
	  -e 's|@PACKAGE\@|@PACKAGE@|g' \
	  -e 's|@VERSION\@|@VERSION@|g' \
	  -e 's|@PACKAGE_REVISION\@|@PACKAGE_REVISION@|g' \
	./ebin/sipxplugin.app > ./ebin/sipxplugin.app.sed

install-exec-hook : sipxplugin.app
	$(INSTALL) -d $(DESTDIR)$(PLUGIN_DIR)/sipxplugin/ebin
	$(INSTALL) ebin/*.beam $(DESTDIR)$(PLUGIN_DIR)/sipxplugin/ebin/
	$(INSTALL) ./ebin/sipxplugin.app.sed $(DESTDIR)$(PLUGIN_DIR)/sipxplugin/ebin/sipxplugin.app
	$(INSTALL) ./deps/erlmongo/ebin/erlmongo.app $(DESTDIR)$(PLUGIN_DIR)/erlmongo.app
	ln -snf $(DESTDIR)$(PLUGIN_DIR)/sipxplugin/ebin/sipxplugin.app $(DESTDIR)$(PLUGIN_DIR)/sipxplugin.app
