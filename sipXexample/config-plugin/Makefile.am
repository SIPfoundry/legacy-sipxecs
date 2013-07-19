include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

SUBDIRS = \
	. \
	test

EXTRA_DIST = \
	$(example_SRC) \
	$(example_RESOURCES)

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipxexample-config.jar
jar_DATA = $(JAR_FILE)

example_SRC = $(shell cd $(srcdir); find src -type f -name '*.java')

plugin_RESOURCES = \
	$(shell cd $(srcdir); find plugin -type f)

src_RESOURCES = \
	sipxplugin.beans.xml

example_PKGS = \
	sipxconfig-neoconf \
	tapestry-annotations \
	tapestry-framework \
	hivemind

example_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(example_PKGS))

$(JAR_FILE) : javac-example $(example_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(plugin_RESOURCES)) \
	  $(call JarInclude,$(srcdir)/src,$(src_RESOURCES))

# style checks java source code coding style agains proper sipxecs conforming. Not nec.
# but you will probably find very useful
style_SRC = $(example_SRC)

precommit : style validate;

validate : src/sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
