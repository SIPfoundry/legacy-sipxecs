include $(top_srcdir)/config/subdir.am

all:
	echo "Hello world"

install:
	echo "Goodbye cruel world"

regression:
	cp -rf $(srcdir)/* ./
	@$(LocalizeSipXconfig) $(srcdir)/copynattraversalrules.sh.in > copynattraversalrules.sh
	@$(LocalizeSipXconfig) $(srcdir)/copysipxbridge.sh.in > copysipxbridge.sh
	$(ANT_CMD) -f build.xml rstart
	$(ANT_CMD) -f build.xml -Dtestname=itsp-local-call-pickup -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml -Dtestname=itsp-local-itsp-hangup-after-transfer -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml -Dtestname=itsp-local-itsp-hangup-during-transfer -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-local-hangup -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml -Dtestname=itsp-forward-itsp -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml -Dtestname=itsp-local-attended-itsp -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml -Dtestname=itsp-local-blind-local -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-attended-itsp -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-attended-local -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-blind-itsp -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-blind-local -Dconf.dir=@SIPX_CONFDIR@
	$(ANT_CMD) -f build.xml rstop

.PHONY: regression
