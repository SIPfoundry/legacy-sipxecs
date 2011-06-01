include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/ruby.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(sipXconfig_report_SRC) \
	sipXconfig-report.gemspec.in

noinst_DATA = \
	sipXconfig-report-$(sipXconfig_report_VER).gem

sipXconfig_report_SRC = $(shell cd $(srcdir); find lib -name '*.rb')
