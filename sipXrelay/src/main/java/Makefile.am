include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

noinst_DATA = javac-relay

EXTRA_DIST = \
	$(relay_SRC)

relay_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

relay_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(relay_PKGS))

jardir = @SIPX_JAVADIR@/sipXbridge
JAR_FILE = sipXrelay.jar
jar_DATA = \
	$(JAR_FILE)

$(JAR_FILE) : javac-relay Manifest.txt
	jar -cfm $@ Manifest.txt -C $(JAVAROOT) .
