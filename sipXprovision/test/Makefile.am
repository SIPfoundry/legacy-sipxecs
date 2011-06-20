include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

noinst_DATA = javac-test
EXTRA_DIST = \
	$(test_SRC)

test_PKGS = \
	$(provision_PKGS) \
	junit 

test_JAVAFLAGS = -Dbasedir=$(srcdir)

test_DEPS = \
	$(JAVAROOT) \
	../src/classes \
	$(srcdir)/../src \
	$(srcdir) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(test_PKGS))

test_SRC = $(shell cd $(srcdir); find org -name '*.java')
