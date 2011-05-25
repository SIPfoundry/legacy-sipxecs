# None of these tests actually work. Disabled from top-level build in parent's Makefile.am --Douglas
java_modules = test

include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(test_SRC) \
	$(test_RESOURCES)

test_SRC = $(shell cd $(srcdir); find . -name '*.java')

test_DEPS = \
	$(JAVAROOT) \
	@SIPX_JAVADIR@/sipXbridge/sipXrelay.jar \
	$(shell find @SIPX_JAVADIR@/sipXcommons -type f -name '*.jar') \
	$(top_builddir)/src/main/java/classes
