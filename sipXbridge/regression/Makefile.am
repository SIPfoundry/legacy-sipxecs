include $(top_srcdir)/config/subdir.am

regression:
	cp -r $(srcdir)/* ./
	@$(LocalizeSipXconfig) $(srcdir)/copynattraversalrules.sh.in > copynattraversalrules.sh
	@$(LocalizeSipXconfig) $(srcdir)/copysipxbridge.sh.in > copysipxbridge.sh
	$(ANT_CMD) -f build.xml rstart
	$(ANT_CMD) -f build.xml -Dtestname=itsp-forward-itsp
	$(ANT_CMD) -f build.xml -Dtestname=itsp-local-attended-itsp
	$(ANT_CMD) -f build.xml -Dtestname=itsp-local-blind-local
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-attended-itsp
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-attended-local
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-blind-itsp
	$(ANT_CMD) -f build.xml -Dtestname=local-itsp-blind-local
	$(ANT_CMD) -f build.xml rstop

.PHONY: regression
