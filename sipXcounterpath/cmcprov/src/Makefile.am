include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(cmcprov_SRC) \
	$(war_RESOURCES)

war_RESOURCES = $(shell cd $(srcdir); find context -type f)

noinst_DATA = javac-cmcprov

wardir = @SIPX_JAVADIR@/sipXconfig
WAR_FILE = cmcprov.war
war_DATA = $(WAR_FILE)

cmcprov_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')

cmcprov_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(cmcprov_PKGS)) \
	../../device/src/classes

cmcprov_JAVAROOT = out/WEB-INF/classes

$(WAR_FILE) : javac-cmcprov $(war_RESOURCES)
	jar -cf $@.tmp \
	  $(call JarInclude,out,.)
	jar -uf $@.tmp \
	  $(call JarInclude,$(srcdir)/context,.)
	mv $@.tmp $@

style_SRC = $(cmcprov_SRC)
style_DEPS = $(call JavaDep,@SIPX_JAVADIR@/sipXcommons,javax.servlet)
precommit : style
