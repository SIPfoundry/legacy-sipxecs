include config/utility.am
include config/project.am
include config/ruby.am
include common.am

@PACKAGE@_EXTRA_DIST = \
	$(sipxcallresolver_SRC) \
	sipxcallresolver.gemspec.in \
	sipxcallresolver.wsdl

SUBDIRS = \
	java-client \
	bin \
	etc \
	.

sipxcallresolver_SRC = $(shell cd $(srcdir); find lib -type f -name '*.rb')

noinst_DATA = \
	sipxcallresolver-$(sipxcallresolver_VER).gem
