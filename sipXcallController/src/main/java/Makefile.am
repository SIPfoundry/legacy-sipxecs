include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

noinst_DATA = javac-callcntlr

EXTRA_DIST = \
	$(callcntlr_SRC) \
	$(callcntlr_RESOURCES)

jardir = @SIPX_JAVADIR@/sipXrest/plugins
JAR_FILE = sipXcallController.jar
jar_DATA = \
	$(JAR_FILE)

callcntlr_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

callcntlr_RESOURCES = \
	plugin.xml

callcntlr_PKGS = \
	  sipxcommons \
	  jain-sip-sdp \
	  org.restlet \
	  log4j

callcntlr_DEPS = \
	$(JAVAROOT) \
	@SIPX_JAVADIR@/sipXrest/sipxrest.jar \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(callcntlr_PKGS))

$(JAR_FILE) : javac-callcntlr Manifest.txt $(callcntlr_RESOURCES)
	jar -cfm $@ Manifest.txt -C $(JAVAROOT) .
	cd $(srcdir); jar -uf $(abspath $@)  $(callcntlr_RESOURCES)
