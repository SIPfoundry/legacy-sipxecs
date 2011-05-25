include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(test_SRC) \
	$(test_RESOURCES)


noinst_DATA = javac-test

test_SRC = $(shell cd $(srcdir); find . -name '*.java')

test_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,junit sipxcommons) \
	$(top_builddir)/src/main/java/classes
