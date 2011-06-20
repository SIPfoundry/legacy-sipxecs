include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/cmcprov/src/common.am

noinst_DATA = javac-test

EXTRA_DIST = \
	$(test_SRC)

test_PKGS = \
	$(cmcprov_PKGS) \
	junit

test_DEPS = \
	$(JAVAROOT) \
	../src/out/WEB-INF/classes \
	$(srcdir)/../src \
	$(srcdir) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig, $(test_PKGS))

test_SRC = $(shell cd $(srcdir); find org -name '*.java')

precommit : check
