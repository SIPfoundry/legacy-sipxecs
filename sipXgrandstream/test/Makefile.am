include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/src/common.am

noinst_DATA = javac-test

test_PKGS = \
	$(grandstream_PKGS) \
	junit \
	org.restlet \
	com.noelios.restlet \
	dbunit \
	easymock \
	easymockclassextension

test_DEPS = \
	$(JAVAROOT) \
	../src/classes \
	$(srcdir)/../src \
	$(srcdir) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig, $(test_PKGS))

test_JAVAFLAGS = -Dclasspath.dir=$(srcdir) -Dbasedir=$(top_builddir)

test_SRC = $(shell cd $(srcdir); find org -name '*.java')
