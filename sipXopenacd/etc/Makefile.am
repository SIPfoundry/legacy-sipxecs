EXTRA_DIST = \
	$(proc_DATA:.xml=.xml.in)

LOCALIZE_VARS = \
	$(filter SIPX%,$(.VARIABLES)) \
	$(filter PACKAGE%,$(.VARIABLES)) \
	$(filter OPENACD%,$(.VARIABLES))

LOCALIZE = \
	$(foreach V,$(LOCALIZE_VARS), -e 's|@$(V)\@|$($(V))|g')

openacd_etcdir = $(SIPX_CONFDIR)/openacd
dist_openacd_etc_DATA = \
	vm.args \
	app.config

procdir = $(SIPX_DATADIR)/process.d

proc_DATA = \
	sipxopenacd-process.xml

$(proc_DATA) : % : %.in
	@sed $(LOCALIZE) $< > $@

install-exec-hook:
	$(INSTALL) -d -m 755 $(DESTDIR)@SIPX_LOGDIR@/openacd $(DESTDIR)@SIPX_VARDIR@/openacd