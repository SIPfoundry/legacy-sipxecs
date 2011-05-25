include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(bridge_SRC)

noinst_DATA = javac-bridge

bridge_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

bridge_PKGS = \
	Stun4J \
	commons-beanutils \
	commons-collections \
	commons-digester \
	commons-logging \
	commons-codec \
	dnsjava \
	jain-sip-sdp \
	javax.servlet \
	jetty \
	log4j \
	sipxcommons \
	ws-commons-util \
	xmlrpc-client \
	xmlrpc-server

bridge_DEPS = \
	$(JAVAROOT) \
	@SIPX_JAVADIR@/sipXbridge/sipXrelay.jar \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(bridge_PKGS))
