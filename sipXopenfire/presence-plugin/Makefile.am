include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am	

SUBDIRS = \
	src

EXTRA_DIST = \
	plugin.xml \
	web \
	$(test_SRC) \
	config.properties.in

#	 javac-test  doesn't compile

jardir = @SIPX_JAVADIR@/sipXopenfire/plugins
jar_DATA = \
	$(presence_JAR)

plugin_JAR = src/sipx-openfire-presence-plugin.jar

# presence jar include other jars. more like a war.
presence_JAR = sipx-openfire-presence.jar
presence_FILES = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(plugin_JAR_RT_PKGS)) \
	$(plugin_JAR)

$(presence_JAR) :
	! test -d lib || rm -rf lib
	mkdir lib
	cp $(presence_FILES) lib
	jar -cf $@ \
	  -C $(srcdir) web \
	  -C $(srcdir) plugin.xml \
	  lib 

clean-local :
	rm -rf lib
	rm -rf $(plugin_JAR)
	rm -rf $(presence_JAR)
	rm -rf config.properties

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
