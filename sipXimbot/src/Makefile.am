include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(imbot_SRC)

noinst_DATA = javac-imbot

jardir = @SIPX_JAVADIR@/sipXimbot
JAR_FILE = sipXimbot.jar
jar_DATA = $(JAR_FILE)

imbot_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')

imbot_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(imbot_PKGS))

$(JAR_FILE) : javac-imbot
	jar -cf $@ -C $(JAVAROOT) .
