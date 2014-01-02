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
	packages/intl \
	packages/logging \
	packages/path \
	packages/unmodifiable_collection

SRC_PUB_PACKAGES = $(addprefix $(srcdir)/,$(PUB_PACKAGES))

SUBDIRS = \
	. \
	bin \
	etc \
	lib \
	report \
	neoconf \
	web

# Running "make" in sipXconfig root, should download dart packages if they aren't 
# already downloaded. 
noinst_DATA = $(SRC_PUB_PACKAGES)

# Notice, this does not detect if newer versions of dependencies are available for download
# If however the file pubspec.yaml is updated to require new versions of deps then dart's 
# package manager should be re-executed.
pub-get $(SRC_PUB_PACKAGES) : pubspec.yaml
	cd $(srcdir); $(DART_HOME)/bin/pub get
	touch $(SRC_PUB_PACKAGES)
	$(MAKE) remove-extraneous-packages-symlinks

# Dart's package manager has an annoying feature that generates packages symlinks in
# web directory.  This removes all the copies that are not needed by how sipXconfig has integrated
# dart.  See following URL for background on links:
#  http://stackoverflow.com/questions/15888308/is-there-a-way-to-disable-all-those-symlinks-to-the-packages-directory
remove-extraneous-packages-symlinks :
	find $(srcdir)/web \
	  -name packages -type l \
	  -not -path '$(srcdir)/web/context/WEB-INF/*' \
	  -exec rm {} \;

install-data-hook : $(SRC_PUB_PACKAGES)
	$(mkdir_p) $(DESTDIR)@SIPX_VARDIR@/upload
	$(mkdir_p) $(DESTDIR)@SIPX_VARDIR@/backup
