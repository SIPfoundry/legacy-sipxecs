include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(cfg_SRC) \
	$(cfg_RESOURCES)
	
noinst_DATA = \
	javac-cfg
	
jardir = @SIPX_JAVADIR@/sipXopenfire/plugins
cfg_JAR = ofsqa-plugin.jar
jar_DATA = $(cfg_JAR)

sqaclient_JAR = sipxsqaclient.jar

cfg_RESOURCES = \
	plugin.xml
                      
cfg_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(plugin_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar

cfg_SRC = \
	$(shell cd $(srcdir); find src -name '*.java')
	  
$(cfg_JAR) : javac-cfg $(cfg_RESOURCES)
	! test -d lib || rm -rf lib
	mkdir lib
	cp @SIPX_JAVADIR@/sipXcommons/$(sqaclient_JAR) lib	
	jar -cf $@ \
	$(JAVAROOT) \
	lib \
	$(call JarInclude,$(srcdir),$(cfg_RESOURCES))