include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(jar_SRC)

noinst_DATA = javac-imbot

jardir = @SIPX_JAVADIR@/sipXimbot/sipXimbot.jar
jar_DATA = $(JAR_FILE)

imbot_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
imbot_PKGS = \
	sipxcommons \
	log4j \
	smackx \
	commons-codec \
	commons-httpclient \
	commons-lang \
	commons-logging \
	jetty \
	javax.servlet \
	xmlrpc-client

imbot_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(imbot_PKGS)) \
	@SIPX_JAVADIR@/sipXopenfire/plugins/sipx-openfire-presence-plugin.jar

$(JAR_FILE) : javac-imbot
	jar -cf $@ -C $(JAVAROOT) .
