include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(provider_SRC)

noinst_DATA = javac-provider

jardir = @SIPX_JAVADIR@/sipXopenfire/lib
jar_DATA = \
	$(provider_JAR)

provider_JAR = sipx-openfire-vcard-provider.jar
provider_SRC = $(shell cd $(srcdir); find org -name '*.java')
provider_DEPS = \
	$(JAVAROOT) \
	@OPENFIRE_HOME@/lib/openfire.jar \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, \
	  ws-commons-util \
	  xmlrpc-client \
	  xmlrpc-server \
	)

$(provider_JAR) : javac-provider
	jar -cf $@ $(JAVAROOT)
