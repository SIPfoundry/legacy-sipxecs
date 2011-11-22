include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(test_SRC)

precommit : check;

noinst_DATA = javac-test

test_PKGS = \
	junit \
	sipxcommons \
	commons-lang

test_DEPS = \
	$(JAVAROOT) \
	../classes \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(test_PKGS))

test_SRC = $(shell cd $(srcdir); find . \( \
	 -name '*.java' \
	\) )
