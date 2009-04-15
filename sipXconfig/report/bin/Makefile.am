include $(top_srcdir)/config/subdir.am

GEM_LIB_DIR=$(libdir)/ruby/gems/1.8

edit = $(LocalizeSipXconfig) \
	-e 's,@GEM_LIB_DIR\@,$(GEM_LIB_DIR),g'

# NOTE: using automake variable naming magic
setupbindir = @SIPX_LIBEXECDIR@/setup.d

setupbin_SCRIPTS = \
	sipxconfig-report-setup

bin_SCRIPTS = \
	sipxconfig-report

sipxconfig-% : sipxconfig-%.in
	@echo "Localize $< -> $@"
	@$(edit) $< > $@
