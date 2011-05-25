include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(cmcprov_SRC) \
	$(war_RESOURCES)

war_RESOURCES = $(shell cd $(srcdir); find context -type f)

noinst_DATA = javac-cmcprov

wardir = @SIPX_JAVADIR@/sipXconfig/cmcprov.war
war_DATA = $(WAR_FILE)

cmcprov_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')

cmcprov_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(cmcprov_PKGS))

cmcprov_JAVAROOT = out/WEB-INF/classes

$(WAR_FILE) : javac-cmcprov $(war_RESOURCES)
	jar -cf $@ \
	-C out . \
	-C $(srcdir)/context .
