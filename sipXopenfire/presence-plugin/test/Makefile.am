include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/../common.am

noinst_DATA = javac-test
test_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
test_PKGS = \
	junit \
	smackx \
	sipxcommon

test_DEPS = \
	../classes \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(test_PKGS)) \
	$(plugin_DEPS)
