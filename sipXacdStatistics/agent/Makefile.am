include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/ruby.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(sipxacd_stats_SRC) \
	sipxacd-stats.gemspec.in \
	sipxacd-stats.wsdl

noinst_DATA = \
	sipxacd-stats-$(sipxacd_stats_VER).gem

sipxacd_stats_SRC = $(shell cd $(srcdir); find lib -name '*.rb')
