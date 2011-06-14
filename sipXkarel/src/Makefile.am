include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(karel_SRC) \
	$(karel_RESOURCES)

noinst_DATA = javac-karel $(JAR_FILE)

JAR_FILE = sipXkarel.jar
jardir = @SIPX_JAVADIR@/sipXconfig/plugins
jar_DATA = $(JAR_FILE)

karel_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
karel_RESOURCES = sipxplugin.beans.xml

karel_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(karel_PKGS))

$(JAR_FILE) : javac-karel $(karel_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(karel_RESOURCES))

style_SRC = $(karel_SRC)
precommit : style
