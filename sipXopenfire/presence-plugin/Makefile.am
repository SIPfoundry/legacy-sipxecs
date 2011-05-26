include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

SUBDIRS = \
	. \
	test

EXTRA_DIST = \
	plugin.xml \
	$(plugin_SRC)

noinst_DATA = javac-plugin

jardir = @SIPX_JAVADIR@/sipXopenfire/plugins
jar_DATA = \
	$(presence_JAR) \
	$(plugin_JAR)

plugin_JAR = sipx-openfire-presence-plugin.jar
plugin_SRC = $(shell cd $(srcdir); find org -name '*.java')
plugin_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(plugin_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar

# presence jar include other jars. more like a war.
presence_JAR = sipx-openfire-presence.jar
presence_FILES = \
	$(plugin_DEPS) \
	$(wildcard @SIPX_JAVADIR@/sipXcommons/libhostname.so*) \
	$(plugin_JAR)

$(presence_JAR) : $(plugin_JAR)
	! test -d lib || rm -rf lib
	mkdir lib
	cp $(presence_FILES) lib
	jar -cf $@ \
	  -C $(srcdir) web \
	  -C $(srcdir) plugin.xml \
	  lib

$(plugin_JAR) : javac-plugin PluginManifest.txt
	 jar -cfm $@ PluginManifest.txt -C $(JAVAROOT) .

PluginManifest.txt : Makefile
	echo "$$plugin_manifest_body" > $@

define plugin_manifest_body
$(manifest_body)
MessageInterceptorClass: org.sipfoundry.openfire.plugin.presence.DefaultMessagePacketInterceptor
endef
export plugin_manifest_body
