include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

WSDL = $(abspath $(srcdir)/../sipxcallresolver.wsdl)

noinst_DATA = javac-client

jardir = @SIPX_JAVADIR@/sipXconfig
JAR_FILE = cdr-bindings.jar
jar_DATA = \
	$(JAR_FILE)

client_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,axis)

# NOTE: This looks in builddir because src is generated
client_SRC = $(shell find org -name '*.java')

javac-client : client-src

client-src : $(WSDL) Makefile
	$(call JavaCommand,client) org.apache.axis.wsdl.WSDL2Java \
	  --NStoPkg urn:CdrService=org.sipfoundry.sipxconfig.cdr \
	  $(WSDL)
	echo timestamp > $@

$(JAR_FILE) : javac-client Manifest.txt
	jar -cfm $@ Manifest.txt -C $(JAVAROOT) .

