EXTRA_DIST = $(bin_SOURCE)

LOCALIZE_VARS = \
	$(filter SIPX%,$(.VARIABLES)) \
	$(filter PACKAGE%,$(.VARIABLES)) \
	$(filter OPENACD%,$(.VARIABLES))

LOCALIZE = \
	$(foreach V,$(LOCALIZE_VARS), -e 's|@$(V)\@|$($(V))|g')

bin_SCRIPTS = \
	sipxopenacd \
	find-erlang-lib

bin_SOURCE = \
	sipxopenacd.in \
	find-erlang-lib

LOCALIZE_TARGETS = \
	$(bin_SCRIPTS)

sipxopenacd :
	@sed $(LOCALIZE) $(srcdir)/$@.in > $@
