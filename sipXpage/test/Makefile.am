include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(test_SRC)

noinst_DATA = javac-test

test_PKGS = \
	$(page_PKGS) \
	junit \
	commons-lang

precommit : check

test_DEPS = \
	$(JAVAROOT) \
	../src/classes \
	$(srcdir)/../src \
	$(srcdir) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(test_PKGS))

# ant port, RestfulRequestTest is not working
test_SRC = $(shell cd $(srcdir); find org \( \
	  -name '*.java' \
	\))
