include config/utility.am
include config/project.am

@PACKAGE@_EXTRA_DIST = \
	$(PUB_PACKAGES)

SUBDIRS = \
  bin \
  etc \
  cpp-executable \
  cpp-library \
  config-plugin \
  ruby-module

PUB_PACKAGES = \
	packages/browser \
	packages/sipxconfig/sipxconfig.dart

pub-get $(PUB_PACKAGES) :
	cd $(srcdir); $(DART_HOME)/bin/pub get

install-data-hook : $(PUB_PACKAGES)
