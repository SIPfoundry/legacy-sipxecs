include $(top_srcdir)/config/utility.am

noinst_SCRIPTS = \
	copynattraversalrules.sh \
	copysipxbridge.sh

EXTRA_DIST = \
	$(noinst_SCRIPTS:=.in)

$(noinst_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

regression: $(noinst_SCRIPTS)
	cp -rf $(srcdir)/* ./
	$(ANT_CMD) -f build.xml rstart
	$(ANT_CMD) -f build.xml -Dconf.dir=@SIPX_CONFDIR@ regression
	$(ANT_CMD) -f build.xml rstop

.PHONY: regression
