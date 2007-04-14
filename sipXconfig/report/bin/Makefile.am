include $(top_srcdir)/config/subdir.am

GEM_LIB_DIR=$(libdir)/ruby/gems/1.8

edit = $(LocalizeSipXconfig) \
	-e 's,@GEM_LIB_DIR\@,$(GEM_LIB_DIR),g'

bin_SCRIPTS = \
	sipxconfig-report

sipxconfig-report : sipxconfig-report.in
	$(edit) $(srcdir)/sipxconfig-report.in > sipxconfig-report
