include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(xtest_SRC) \
	$(xtest_RESOURCES)

# No actually usable tests here so renamed from javax-test
noinst_DATA = javac-xtest

xtest_SRC = $(shell cd $(srcdir); find org -name '*.java')

xtest_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,junit $(rest_PKGS)) \
	$(top_builddir)/src/classes
