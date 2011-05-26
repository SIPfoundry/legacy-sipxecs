include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(test_SRC) \
	$(test_RESOURCES)

noinst_DATA = javac-test

test_SRC = $(shell cd $(srcdir); find org -name '*.java')
test_RESOURCES = $(shell find $(srcdir) -type f -not -name '*.java')

test_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, junit $(sipxacc_PKGS)) \
	$(sipxacc_JAVAROOT)
