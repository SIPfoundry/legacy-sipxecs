include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(provider_SRC)

noinst_DATA = javac-provider

jardir = @SIPX_JAVADIR@/sipXopenfire/lib

jar_PKGS = \
	log4j \
	mongo \
	ws-commons-util \
	xmlrpc-common \
	xmlrpc-client \
	xmlrpc-server

# copy jars into openfire lib dir. NOTE: I did not verify this is nec. but preserved from ant port
# but it wouldn't seem like is nec.  Openfire integration i think would benefit from a full 
# review. --douglas
# not necessary to copy jars into openfire lib - only sipxecs jars that need to be deployed in openfire lib (/opt/openfire/lib) should be put here
# but not jars from sipXcomons. sipxopenfire.in specifically creates symbolic links with needed jars from sipXcommons --mircea
jar_DATA = \
	$(provider_JAR)

provider_JAR = sipx-openfire-vcard-provider.jar
provider_SRC = $(shell cd $(srcdir); find src -name '*.java')
provider_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(provider_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar \
	../vcard-synchserver/sipx-openfire-vcard-synchserver.jar

$(provider_JAR) : javac-provider Manifest.txt Makefile
	jar cfm $@ Manifest.txt \
	  $(call JarInclude,$(sipxvcard_JAVAROOT),.) \
	  $(call JarInclude,../presence-plugin,config.properties)
