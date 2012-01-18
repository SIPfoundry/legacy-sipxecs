include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

WSDL = $(abspath $(srcdir)/../agent/sipxacd-stats.wsdl)

noinst_DATA = javac-client

jardir = @SIPX_JAVADIR@/sipXconfig
JAR_FILE = agent-bindings.jar
jar_DATA = \
	$(JAR_FILE)

client_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,axis)

client_SRCDIR = src
client_SRC = $(shell find $(client_SRCDIR) -name '*.java')

wsdl-client : $(WSDL)
	test -d $(client_SRCDIR) || mkdir $(client_SRCDIR)
	$(call JavaCommand,client) org.apache.axis.wsdl.WSDL2Java \
	  --NStoPkg urn:StatsService=org.sipfoundry.sipxconfig.acd.stats \
	  --output $(client_SRCDIR) \
	  $(WSDL)
	@echo timestamp > $@

CLEANFILES = wsdl-client
wsdl-client-clean :
	! test -d $(client_SRCDIR) || rm -rf $(client_SRCDIR)

javac-client-clean : wsdl-client-clean

javac-client : wsdl-client

$(JAR_FILE) : javac-client Manifest.txt
	jar -cfm $@ Manifest.txt -C $(JAVAROOT) .
