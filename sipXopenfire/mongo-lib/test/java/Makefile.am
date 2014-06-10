include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

noinst_DATA = javac-test

EXTRA_DIST = \
	$(test_SRC) \
	$(test_RESOURCES)

test_SRC = $(shell cd $(srcdir); find . \( \
	-name '*.java' \
	\))

test_RESOURCES = $(shell cd $(srcdir); find . \( \
	-type f \
	-not -name '*.java' \
	-not -name 'README.TXT' \
	\))

test_PKGS = \
	$(plugin_PKGS) \
	junit

test_DEPS = \
	$(srcdir) \
	$(JAVAROOT) \
	$(mongoplugin_JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(test_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar \
	@OPENFIRE_HOME@/lib/slf4j-log4j12.jar \
	@SIPX_JAVADIR@/sipXopenfire/lib/sipx-openfire-vcard-synchserver.jar

precommit : check
