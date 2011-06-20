.DELETE_ON_ERROR:

EXTRA_DIST = $(in_files)
DISTCLEANFILES = $(out_files)

out_files = \
	$(fssetupbin_SCRIPTS) \
	$(bin_SCRIPTS)

in_files = \
	$(out_files:.sh=.sh.in)

LOCALIZE_VARS = \
	$(filter SIPX%,$(.VARIABLES)) \
	$(filter PACKAGE%,$(.VARIABLES)) \
	$(filter FREESWITCH%,$(.VARIABLES))

LOCALIZE = \
	$(foreach V,$(LOCALIZE_VARS), -e 's|@$(V)\@|$($(V))|g')

fssetupbindir = @SIPX_LIBEXECDIR@/setup.d
fssetupbin_SCRIPTS = \
    freeswitch_setup.sh

bin_SCRIPTS = \
    freeswitch.sh

$(bin_SCRIPTS) $(fssetupbin_SCRIPTS) : % : %.in Makefile
	@sed $(LOCALIZE) $< > $@
