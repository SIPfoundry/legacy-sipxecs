include config/project.am

SUBDIRS = src bin

@PACKAGE@_EXTRA_DIST = \
	README \
	LICENSE	\
	sipxopenacd-process.xml.in

LOCALIZE_VARS = \
	$(filter SIPX%,$(.VARIABLES)) \
	$(filter PACKAGE%,$(.VARIABLES)) \
	$(filter OPENACD%,$(.VARIABLES))

LOCALIZE = \
	$(foreach V,$(LOCALIZE_VARS), -e 's|@$(V)\@|$($(V))|g')

procdir = $(datadir)/sipxecs/process.d

dist_proc_DATA = \
	sipxopenacd-process.xml

LOCALIZE_TARGETS = \
	$(dist_proc_DATA)

# source files use many configuration variables that change, so always rebuild
.PHONY: $(LOCALIZE_TARGETS)
$(LOCALIZE_TARGETS) :
	@sed $(LOCALIZE) $(srcdir)/$(notdir $@).in > $@
