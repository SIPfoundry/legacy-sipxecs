include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(cfg_SRC) \
	$(cfg_RESOURCES)

# tests do not compile
#SUBDIRS = \
#	. \
#	test

noinst_DATA = javac-cfg

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
jar_DATA = $(cfg_JAR)
cfg_JAR = cfg-service.jar
cfg_RESOURCES = sipxplugin.beans.xml

$(cfg_JAR) : javac-cfg
	jar -cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(cfg_RESOURCES))

cfg_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(cfg_PKGS))

cfg_SRC = \
	$(shell cd $(srcdir); find org -name '*.java')
