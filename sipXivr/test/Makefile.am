include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(test_SRC)

noinst_DATA = javac-test

test_PKGS = \
	$(ivr_PKGS) \
	junit \
	mongo

test_DEPS = \
	$(JAVAROOT) \
	../src/classes \
	$(srcdir)/../src \
	$(srcdir) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(test_PKGS))

# ant port, RestfulRequestTest is not working
test_SRC = $(shell cd $(srcdir); find org \( \
	-name '*.java' \
	-a -not -name 'RestfulRequestTest.java' \
	-a -not -name 'MessageTest.java' \
	\))

precommit : check
