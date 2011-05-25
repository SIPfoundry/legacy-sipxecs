include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(polycom_SRC) \
	$(polycom_RESOURCES)

noinst_DATA = javac-polycom

jardir = @SIPX_JAVADIR@/sipXconfig
JAR_FILE = sipXpolycom.jar
jar_DATA = $(JAR_FILE)

polycom_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
polycom_RESOURCES = sipxplugin.beans.xml

polycom_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(polycom_PKGS))

$(JAR_FILE) : javac-polycom $(polycom_RESOURCES)
	jar cf $@.tmp -C $(JAVAROOT) .
	cd $(srcdir); jar uf $(abspath $@.tmp) $(polycom_RESOURCES)
	mv $@.tmp $@
