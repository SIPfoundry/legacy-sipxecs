include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(cfg_SRC)
	
noinst_DATA = \
	javac-cfg
	
cfg_JAR = sipx-ofmongo-lib.jar
jar_DATA = $(cfg_JAR)
jardir = @SIPX_JAVADIR@/sipXopenfire/lib
                      
cfg_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(plugin_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar \
	@SIPX_JAVADIR@/sipXopenfire/lib/sipx-openfire-vcard-synchserver.jar

cfg_SRC = \
	$(shell cd $(srcdir); find . -name '*.java')
	  
$(cfg_JAR) : javac-cfg
	jar -cf $@ \
	  $(call JarInclude,$(mongolib_JAVAROOT),.)