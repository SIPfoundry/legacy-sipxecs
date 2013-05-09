include config/utility.am
include config/project.am
include config/java.am
include common.am

TEST_DB = SIPXCONFIG_TEST

@PACKAGE@_EXTRA_DIST = \
	@PACKAGE@-rpmlintrc \
	LICENSE \
	NOTICE \
	packages \
	meta

SUBDIRS = \
	bin \
	etc \
	lib \
	report \
	neoconf \
	web \
	.

install-data-hook :
	$(mkdir_p) $(DESTDIR)@SIPX_VARDIR@/upload
	$(mkdir_p) $(DESTDIR)@SIPX_VARDIR@/backup
