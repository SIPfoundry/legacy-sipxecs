include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(counterpath_SRC) \
	$(counterpath_RESOURCES)

noinst_DATA = javac-counterpath

jardir = @SIPX_JAVADIR@/sipXconfig
JAR_FILE = sipXcounterpath.jar
jar_DATA = $(JAR_FILE)

counterpath_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
counterpath_RESOURCES = sipxplugin.beans.xml

counterpath_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(counterpath_PKGS))

$(JAR_FILE) : javac-counterpath $(counterpath_RESOURCES)
	jar cf $@.tmp -C $(JAVAROOT) .
	cd $(srcdir); jar uf $(abspath $@.tmp) $(counterpath_RESOURCES)
	mv $@.tmp $@
