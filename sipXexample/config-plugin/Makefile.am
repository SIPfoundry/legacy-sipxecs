include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/config/sipxconfig.am

SUBDIRS = \
	. \
	test

EXTRA_DIST = \
	$(example_SRC) \
	plugin \
	src/sipxplugin.beans.xml

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipxexample-config.jar
jar_DATA = $(JAR_FILE)

example_SRC = $(shell cd $(srcdir); find src -type f -name '*.java')

plugin_RESOURCES = \
	$(shell cd $(srcdir); find plugin -type f)

example_PKGS = \
	$(sipxconfig_web_PKGS) \
	sipxconfig-neoconf \
	sipxconfig-web

example_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(example_PKGS))

$(JAR_FILE) : javac-example java-dart $(example_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(plugin_RESOURCES)) \
	  $(call JarInclude,$(srcdir)/src,sipxplugin.beans.xml)

# file in packages get compiled in thru compiling page dart files w/main()s
DART_SRC = $(shell cd $(srcdir); find plugin -name '*.dart' -not -regex '.*/packages/.*')

java-dart : $(addprefix $(JAVAROOT)/,$(DART_SRC:.dart=.js))

$(addprefix $(JAVAROOT)/,$(DART_PKGS)) : $(DART_PKGS)
	test -d $(dir $@) || mkdir -p $(dir $@)
	cp $< $@

$(addprefix $(JAVAROOT)/,$(DART_SRC:.dart=.js)) : $(JAVAROOT)/%.js : %.dart $(wildcard $(top_srcdir)/packages/*/*.dart)
	test -d $(dir $@) || mkdir -p $(dir $@)
	$(DART_HOME)/bin/dart2js -o$@ $<


# style checks java source code coding style agains proper sipxecs conforming. Not nec.
# but you will probably find very useful
style_SRC = $(example_SRC)

precommit : style validate;

validate : src/sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
