include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(cfg_SRC) \
	$(cfg_RESOURCES) \
	$(test_SRC)

# tests do not compile
noinst_DATA = \
	javac-cfg	
# javac-test   tests do not compile	

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
jar_DATA = $(cfg_JAR)
cfg_JAR = openfire-cfg-service.jar
cfg_RESOURCES = \
	$(shell cd $(srcdir); find plugin -type f) \
	sipxplugin.beans.xml

$(cfg_JAR) : javac-cfg $(cfg_RESOURCES)
	jar -cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(cfg_RESOURCES))

cfg_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(cfg_PKGS))

cfg_SRC = \
	$(shell cd $(srcdir); find src -name '*.java')

noinst_DATA = 
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
	