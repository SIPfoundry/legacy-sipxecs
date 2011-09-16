include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/ruby.am

EXTRA_DIST = \
	$(sipxexample_SRC) \
	sipxexample.gemspec.in

sipxexample_VER = 1.0.0

noinst_DATA = \
	sipxexample-$(sipxexample_VER).gem

sipxexample_SRC = $(shell cd $(srcdir); find lib -name '*.rb')
