include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	plugin.xml \
	web \
	$(plugin_SRC) \
	$(test_SRC) \
	config.properties.in

noinst_DATA = \
	javac-plugin

#	 javac-test  doesn't compile

jardir = @SIPX_JAVADIR@/sipXopenfire/plugins
jar_DATA = \
	$(presence_JAR)

plugin_JAR = sipx-openfire-presence-plugin.jar
plugin_SRC = $(shell cd $(srcdir); find src -name '*.java')
plugin_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(plugin_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar

# presence jar include other jars. more like a war.
presence_JAR = sipx-openfire-presence.jar
presence_FILES = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(plugin_JAR_RT_PKGS)) \
	$(plugin_JAR)

config.properties : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

$(presence_JAR) : $(plugin_JAR)
	! test -d lib || rm -rf lib
	mkdir lib
	cp $(presence_FILES) lib
	jar -cf $@ \
	  -C $(srcdir) web \
	  -C $(srcdir) plugin.xml \
	  lib

$(plugin_JAR) : javac-plugin config.properties Manifest.txt
	jar cfm $@ Manifest.txt \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,./,config.properties)

clean-local :
	rm -rf lib

test_JAVAROOT = test-classes

test_SRC = $(shell cd $(srcdir); find test -type f -name '*.java')
test_PKGS = \
	$(plugin_PKGS) \
	junit \
	smackx

test_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(test_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar
