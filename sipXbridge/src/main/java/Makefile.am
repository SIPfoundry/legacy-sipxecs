include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(bridge_SRC)

noinst_DATA = javac-bridge

bridge_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

bridge_DEPS = \
	$(JAVAROOT) \
	@SIPX_JAVADIR@/sipXbridge/sipXrelay.jar \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(bridge_PKGS))
