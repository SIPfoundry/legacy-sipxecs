include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(sync_SRC) \
	plugin.xml

noinst_DATA = javac-sync

sync_JAR = sipx-mongo-sync-plugin.jar
jar_DATA = $(sync_JAR)
jardir = @SIPX_JAVADIR@/sipXopenfire/plugins

jar_PKGS = \
	log4j \
	commons-collections \
	spring-data-mongodb \
	mongo \
	sipxcommons
	
sync_SRC = $(shell cd $(srcdir); find src -name '*.java')
sync_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(jar_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar \
	$(mongolib_JAVAROOT)

$(sync_JAR) : javac-sync
	jar -cf $@ \
	  $(call JarInclude,$(mongosync_JAVAROOT),classes) \
	  $(call JarInclude,$(srcdir),plugin.xml)
