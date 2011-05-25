include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(jar_SRC)

noinst_DATA = javac-audiocodes

jardir = @SIPX_JAVADIR@/sipXconfig
jar_DATA = $(JAR_FILE)

audiocodes_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')

audiocodes_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(audiocodes_PKGS))

$(JAR_FILE) : javac-audiocodes
	jar -cf $@ -C $(JAVAROOT) .
