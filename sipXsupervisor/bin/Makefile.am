include $(top_srcdir)/config/subdir.am

bin_SCRIPTS = \
	sipxsupervisor.sh

sipxsupervisor.sh: sipxsupervisor.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

EXTRA_DIST = sipxsupervisor.sh.in
