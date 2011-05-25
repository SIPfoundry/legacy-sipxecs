include $(top_srcdir)/config/project.am
include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/ruby.am

@PACKAGE@_EXTRA_DIST = \
	$(callresolver_SRC)

SUBDIRS = \
	java-client \
	.

callresolver_SRC = $(shell cd $(srcdir); find lib -type f -name '*.rb')
callresolver_VER = 1.0.0

noinst_DATA = \
	callresolver-$(callresolver_VER).gem
