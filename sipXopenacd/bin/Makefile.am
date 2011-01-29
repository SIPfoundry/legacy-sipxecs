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
	sipxopenacd-setup.in \
	find-erlang-lib

sipxopenacd_setupdir = \
	$(SIPX_LIBEXECDIR)/setup.d

sipxopenacd_setup_SCRIPTS = \
	sipxopenacd-setup

sipxopenacd sipxopenacd-setup : % : %.in
	@sed $(LOCALIZE) $(srcdir)/$@.in > $@
