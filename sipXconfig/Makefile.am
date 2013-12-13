include config/utility.am
include config/project.am
include config/java.am
include common.am

TEST_DB = SIPXCONFIG_TEST

@PACKAGE@_EXTRA_DIST = \
	@PACKAGE@-rpmlintrc \
	LICENSE \
	NOTICE \
	$(PUB_PACKAGES) \
	packages/sipxconfig/sipxconfig.dart \
	meta

PUB_PACKAGES = \
	packages/browser \
	packages/analyzer \
	packages/args \
	packages/collection_helpers \
	packages/intl \
	packages/logging \
	packages/path \
	packages/unmodifiable_collection

SUBDIRS = \
	bin \
	etc \
	lib \
	report \
	neoconf \
	web \
	.

pub-get $(PUB_PACKAGES) :
	cd $(srcdir); $(DART_HOME)/bin/pub get

install-data-hook : $(PUB_PACKAGES)
	$(mkdir_p) $(DESTDIR)@SIPX_VARDIR@/upload
	$(mkdir_p) $(DESTDIR)@SIPX_VARDIR@/backup
