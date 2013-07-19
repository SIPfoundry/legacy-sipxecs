include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/config/sipxconfig-integration-testing.am

EXTRA_DIST = \
	$(test_SRC)

precommit : check
	$(MAKE) sipxconfig-reset-integration-test check-integration

noinst_DATA = javac-test

test_PKGS = \
	$(sipxconfig_test_integration_PKGS)

test_DEPS = \
	$(JAVAROOT) \
	../classes \
	$(srcdir) \
	$(srcdir)/../src \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(test_PKGS))

test_SRC = $(shell cd $(srcdir); find . \( \
	 -name '*.java' \
	\) )
