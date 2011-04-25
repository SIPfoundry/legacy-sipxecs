include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
	$(shell_scripts:=.in) \
	$(setup_SCRIPTS:=.in) \
	$(ruby_scripts:=.rb)

bin_SCRIPTS = \
	$(shell_scripts) \
	$(ruby_scripts)

shell_scripts = \
	sipx-mrtg-init \
	sipx-snmp-init \
	sipx-time-manager \
	sipx-tunnel

ruby_scripts = \
	sipx-upgrade-3.6-mailstore \
	sipx-upgrade-3.6-mailstore-report

setupdir = @SIPX_LIBEXECDIR@/setup.d
setup_SCRIPTS = \
	sipxconfig-setup

$(ruby_scripts) : % : %.rb
	$(info "Localize $< -> $@")
	@${LocalizeSipXconfig} $< > $@

$(setup_SCRIPTS)    $(shell_scripts) : % : %.in
	$(info "Localize $< -> $@")
	@$(LocalizeSipXconfig) \
	  -e 's,@MRTG_BINARY\@,@MRTG_BINARY@,g' \
	  -e 's,@DATE_BINARY\@,@DATE_BINARY@,g' $< > $@
