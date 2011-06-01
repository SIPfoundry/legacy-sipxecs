include config/utility.am
include $(top_srcdir)/config/project.am
include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/ruby.am
include $(top_srcdir)/common.am

@PACKAGE@_EXTRA_DIST = \
	$(sipxcallresolver_SRC)

SUBDIRS = \
	java-client \
	bin \
	etc \
	.

sipxcallresolver_SRC = $(shell cd $(srcdir); find lib -type f -name '*.rb')

noinst_DATA = \
	sipxcallresolver-$(sipxcallresolver_VER).gem
