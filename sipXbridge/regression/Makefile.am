include $(top_srcdir)/config/subdir.am

regression:
	cp -r $(srcdir)/* ./
	$(ANT_CMD) -f build.xml rstart
	@$(LocalizeSipXconfig) $(srcdir)/itsp-forward-itsp/sipxbridge.xml.in > /tmp/sipxbridge.xml
	$(ANT_CMD) -f build.xml -Dtestname=itsp-forward-itsp
	@$(LocalizeSipXconfig) $(srcdir)/itsp-local-attended-itsp/sipxbridge.xml.in > /tmp/sipxbridge.xml
	$(ANT_CMD) -f build.xml -Dtestname=itsp-local-attended-itsp
	@$(LocalizeSipXconfig) $(srcdir)/itsp-local-blind-local/sipxbridge.xml.in > /tmp/sipxbridge.xml
	$(ANT_CMD) -f build.xml -Dtestname=itsp-local-blind-local
	@$(LocalizeSipXconfig) $(srcdir)/local-itsp-attended-itsp/sipxbridge.xml.in > /tmp/sipxbridge.xml
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-attended-itsp
	@$(LocalizeSipXconfig) $(srcdir)/local-itsp-attended-local/sipxbridge.xml.in > /tmp/sipxbridge.xml
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-attended-local
	@$(LocalizeSipXconfig) $(srcdir)/local-itsp-blind-itsp/sipxbridge.xml.in > /tmp/sipxbridge.xml
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-blind-itsp
	@$(LocalizeSipXconfig) $(srcdir)/local-itsp-blind-local/sipxbridge.xml.in > /tmp/sipxbridge.xml
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-blind-local
	$(ANT_CMD) -f build.xml rstop

.PHONY: regression
