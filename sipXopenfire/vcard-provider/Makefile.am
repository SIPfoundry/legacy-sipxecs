include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(provider_SRC)

noinst_DATA = javac-provider

jardir = @SIPX_JAVADIR@/sipXopenfire/lib

# copy jars into openfire lib dir. NOTE: I did not verify this is nec. but preserved from ant port
# but it wouldn't seem like is nec.  Openfire integration i think would benefit from a full 
# review. --douglas
jar_PKGS = \
	log4j \
	mongo \
	ws-commons-util \
	xmlrpc-common \
	xmlrpc-client \
	xmlrpc-server

jar_DATA = \
	$(provider_JAR) \
	$(foreach P,$(jar_PKGS),$(wildcard @SIPX_JAVADIR@/sipXcommons/$(P)*.jar))

provider_JAR = sipx-openfire-vcard-provider.jar
provider_SRC = $(shell cd $(srcdir); find org -name '*.java')
provider_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(provider_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar

$(provider_JAR) : javac-provider
	jar -cf $@ $(JAVAROOT)
