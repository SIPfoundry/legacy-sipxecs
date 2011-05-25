include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(linksys_SRC) \
	$(linksys_RESOURCES)

noinst_DATA = javac-linksys

jardir = @SIPX_JAVADIR@/sipXconfig
JAR_FILE = sipXlinksys.jar
jar_DATA = $(JAR_FILE)

linksys_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
linksys_RESOURCES = sipxplugin.beans.xml

linksys_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(linksys_PKGS))

$(JAR_FILE) : javac-linksys $(linksys_RESOURCES)
	jar cf $@.tmp -C $(JAVAROOT) .
	cd $(srcdir); jar uf $(abspath $@.tmp) $(linksys_RESOURCES)
	mv $@.tmp $@
