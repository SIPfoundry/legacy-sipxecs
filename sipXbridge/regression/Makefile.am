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
	$(ANT_CMD) -f build.xml -Dconf.dir=@SIPX_CONFDIR@ regression
	$(ANT_CMD) -f build.xml rstop

.PHONY: regression
