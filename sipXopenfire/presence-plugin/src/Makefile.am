include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(plugin_SRC)

noinst_DATA = \
	javac-plugin
	
plugin_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
plugin_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(plugin_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar
	
plugin_JAR = sipx-openfire-plugin.jar

jardir = @SIPX_JAVADIR@/sipXcommons
jar_DATA = $(plugin_JAR)

../config.properties : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

$(plugin_JAR) : javac-plugin ../config.properties Manifest.txt
	jar cfm $@ Manifest.txt \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,../,config.properties) 