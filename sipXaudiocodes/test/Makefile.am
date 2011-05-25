include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

noinst_DATA = javac-test
EXTRA_DIST = $(test_SRC)

test_PKGS = \
	$(audiocodes_PKGS) \
	junit \
	dbunit

test_JAVAFLAGS = -Dclasspath.dir=$(JAVAROOT)

test_DEPS = \
	$(JAVAROOT) \
	../src/classes \
	$(srcdir)/../src \
	$(srcdir) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(test_PKGS))

test_SRC = $(shell cd $(srcdir); find org -name '*.java')
