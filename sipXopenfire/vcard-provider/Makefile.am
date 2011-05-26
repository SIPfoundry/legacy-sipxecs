include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

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
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons,$(provider_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar

$(provider_JAR) : javac-provider
	jar -cf $@ $(JAVAROOT)
