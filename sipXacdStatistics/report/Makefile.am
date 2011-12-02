include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/ruby.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(sipxacd_report_SRC) \
	sipxacd-report.gemspec.in

noinst_DATA = \
	sipxacd-report-$(sipxacd_report_VER).gem

sipxacd_report_SRC = $(shell cd $(srcdir); find lib -name '*.rb')
