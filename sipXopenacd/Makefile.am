include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/project.am
include $(top_srcdir)/common.am

noinst_DATA = deps/erlmongo deps/meck $(LINK_TARGETS)

SUBDIRS = \
	. \
	etc \
	bin

@PACKAGE@_EXTRA_DIST = \
	$(SRC_FILES) \
	deps \
	src/sipxplugin.app.src \
	rebar \
	rebar.config \
	README \
	LICENSE

deps/.downloaded :
	./rebar get-deps update-deps
	rm -f deps/erlmongo/erlmongo.spec
	touch $@

# C O M P I L E   S R C
# Normally there's a makefile in src to compile but we leverage
# a trick to symlink all of the src files to the build directory
# to comply with a rebar limitation that it's incompatible with
# Automake's VPATH concept.

ERL_FILES = $(shell cd $(srcdir)/src; find ./ -name '*.erl')
SRC_FILES = $(addprefix src/,$(ERL_FILES))
BEAM_FILES = $(addprefix ebin/,$(ERL_FILES:.erl=.beam))

LINK_TARGETS = $(addprefix $(abspath $(builddir))/, src rebar rebar.config)

# rebar wants to mix source and binaries in the same dir so if
# srcdir != bindir then we symlink them from srcdir. Don't copy as
# that can only introduce stale files. Also, this target is safely 
# ignored when srcdir == bindir.
$(SRC_FILES) : $(LINK_TARGETS) deps/.downloaded;
$(LINK_TARGETS) :
	ln -snf $(abspath $(srcdir))/$(notdir $@) $@

# this ensures any change to source then a recompile is triggered
ebin/%.beam : src/%.erl
	ERL_LIBS=@OPENACD_DIR@/lib ./rebar compile skip_deps=true

ebindir = $(SIPX_LIBDIR)/openacd/plugin.d/sipxplugin/ebin
ebin_DATA = \
	$(BEAM_FILES)

plugindir = $(SIPX_LIBDIR)/openacd/plugin.d/sipxplugin/ebin
plugin_DATA = \
	sipxplugin.app

sipxplugin.app : %  : src/%.src Makefile
	@$(call SearchAndReplace,$<,$@,$(OPENACD_VARS))

clean : $(LINK_TARGETS)
	./rebar clean

install-exec-hook : install-meck install-erlmongo;

deps/meck/ebin/meck.beam : Makefile
	cd deps/meck; make REBAR=$(abspath ./rebar)

deps/erlmongo/ebin/erlmongo.beam : Makefile
	cd deps/erlmongo; erl -make

# NOTE: make didn't like "install-% : deps/%/ebin/%.beam"
install-meck : deps/meck/ebin/meck.beam
install-erlmongo : deps/erlmongo/ebin/erlmongo.beam
install-% :
	$(INSTALL) -d $(DESTDIR)$(SIPX_LIBDIR)/openacd/lib/$*/ebin
	$(INSTALL) -d $(DESTDIR)$(SIPX_LIBDIR)/openacd/plugin.d
	$(INSTALL) -t $(DESTDIR)$(SIPX_LIBDIR)/openacd/lib/$*/ebin $(wildcard deps/$*/ebin/*)
