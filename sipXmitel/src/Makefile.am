include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(mitel_SRC) \
	$(mitel_RESOURCES)

noinst_DATA = javac-mitel

jardir = @SIPX_JAVADIR@/sipXconfig
JAR_FILE = sipXmitel.jar
jar_DATA = $(JAR_FILE)

mitel_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
mitel_RESOURCES = sipxplugin.beans.xml

mitel_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(mitel_PKGS))

$(JAR_FILE) : javac-mitel $(mitel_RESOURCES)
	jar cf $@.tmp -C $(JAVAROOT) .
	cd $(srcdir); jar uf $(abspath $@.tmp) $(mitel_RESOURCES)
	mv $@.tmp $@
