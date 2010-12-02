EXTRA_DIST = \
	README \
	CONTRIBUTORS \
	LICENSE	\
	sipxopenacd.spec \
	sipxopenacd-process.xml.in \
	sipxopenacd.in

LOCALIZE_VARS = \
	$(filter SIPX%,$(.VARIABLES)) \
	$(filter PACKAGE%,$(.VARIABLES)) \
	$(filter OPENACD%,$(.VARIABLES))

LOCALIZE = \
	$(foreach V,$(LOCALIZE_VARS), -e 's|@$(V)\@|$($(V))|g')

bin_SCRIPTS = \
	sipxopenacd

procdir = $(datadir)/sipxecs/process.d

dist_proc_DATA = \
	sipxopenacd-process.xml

LOCALIZE_TARGETS = \
	$(bin_SCRIPTS) \
	$(dist_proc_DATA) \
	sipxopenacd.spec

# source files use many configuration variables that change, so always rebuild
.PHONY: $(LOCALIZE_TARGETS)
$(LOCALIZE_TARGETS) :
	sed $(LOCALIZE) $(srcdir)/$(notdir $@).in > $@
