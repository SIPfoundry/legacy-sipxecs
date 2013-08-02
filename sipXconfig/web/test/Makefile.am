include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/config/sipxconfig-integration-testing.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(test_SRC)

if TEST_MODE
jardir = @SIPX_JAVADIR@/sipXconfig/plugins
jar_DATA = sipXconfig-test.jar
jar_CLASSES = \
	org/sipfoundry/sipxconfig/site/InitializeTestSystem.class
$(jar_DATA) : javac-test $(dist_jar_SRC)
	jar cf $@ \
	  $(call JarInclude,$(srcdir),$(dist_jar_SRC)) \
	  $(call JarInclude,$(JAVAROOT),$(jar_CLASSES))
endif

dist_jar_SRC = \
	log4j.properties \
	sipxplugin.beans.xml \
	skin.properties \
	org/sipfoundry/sipxconfig/components/test-logo.png \
	org/sipfoundry/sipxconfig/components/unused.png

noinst_DATA = javac-test

test_PKGS = \
	$(web_PKGS) \
	cssparser \
	dbunit \
	easymock \
	easymockclassextension \
	htmlunit \
	htmlunit-core-js \
	js \
	junit \
	jwebunit \
	jwebunit-core \
	jwebunit-htmlunit-plugin \
	nekohtml \
	p6spy \
	regexp \
	sac \
	slf4j-api \
	slf4j-log4j12

#test_PKGS = $(sipxconfig_test_integration_core_PKGS)

test_DEPS = \
	$(JAVAROOT) \
	$(srcdir) \
	$(neoconf_SRCDIR) \
	$(neoconf_JAVAROOT) \
	$(web_SRCDIR) \
	$(web_JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons $(top_srcdir)/lib,$(test_PKGS))

test_SRC = \
	$(shell cd $(srcdir); find org \( \
	  -name '*.java' \
	\))

precommit : check
