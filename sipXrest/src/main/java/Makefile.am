include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(sipxrest_SRC)

noinst_DATA = javac-rest

jardir = @SIPX_JAVADIR@/sipXrest
JAR_FILE = sipxrest.jar
jar_DATA = \
	$(JAR_FILE)

rest_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

rest_PKGS = \
	com.noelios.restlet \
	com.noelios.restlet.ext.servlet \
	commons-beanutils \
	commons-codec \
	commons-collections \
	commons-digester \
	commons-logging \
	dnsjava \
	jain-sip-sdp \
	javax.servlet \
	jetty \
	log4j \
	org.restlet \
	sipxcommons \
	ws-commons-util \
	xmlrpc-client \
	xmlrpc-server

rest_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(rest_PKGS))

$(JAR_FILE) : javac-rest Manifest.txt
	jar -cfm $@ Manifest.txt -C $(JAVAROOT) .
