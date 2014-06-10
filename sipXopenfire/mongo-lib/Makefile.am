include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(cfg_SRC) \
	$(test_SRC)
	
noinst_DATA = \
	javac-cfg
	
cfg_JAR = sipx-ofmongo-lib.jar
jar_DATA = $(cfg_JAR)
jardir = @SIPX_JAVADIR@/sipXopenfire/lib
                      
cfg_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(plugin_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar \
	${vcardsynchserver_JAVAROOT}/sipx-openfire-vcard-synchserver.jar

cfg_SRC = \
	$(shell cd $(srcdir); find src -name '*.java')
	  
$(cfg_JAR) : javac-cfg
	jar -cf $@ \
	  $(call JarInclude,$(mongolib_JAVAROOT),.)

test_JAVAROOT = classes.test
test_PKGS = \
	$(openfire_PKGS) \
	junit \
	easymock \
	commons-io \
	sipxcommons

test_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(test_PKGS))

test_SRC = $(shell cd $(srcdir); find test -name '*.java')
