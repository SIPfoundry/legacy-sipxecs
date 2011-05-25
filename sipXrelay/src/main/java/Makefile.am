include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

noinst_DATA = javac-relay

EXTRA_DIST = \
	$(relay_SRC)

relay_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

relay_PKGS = \
	Stun4J \
	commons-beanutils \
	commons-collections \
	commons-digester \
	commons-logging \
	dnsjava \
	jain-sip-sdp \
	javax.servlet \
	jetty \
	log4j \
	sipxcommons \
	ws-commons-util \
	xmlrpc-client \
	xmlrpc-server

relay_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(relay_PKGS))
