include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(preflight_SRC)

noinst_DATA = javac-preflight

jardir = @SIPX_JAVADIR@/preflight
JAR_FILE = preflight.jar
jar_DATA = $(JAR_FILE)

preflight_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')

preflight_PKGS = \
	sipxcommons \
	commons-cli \
	commons-net \
	dnsjava \
	jain-sip-sdp \
	log4j \
	sipxcommons

preflight_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(preflight_PKGS))

$(JAR_FILE) : javac-preflight
	jar cf $@ -C $(JAVAROOT) .
