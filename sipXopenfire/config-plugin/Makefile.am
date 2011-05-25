include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(cfg_SRC)

# tests do not compile
#SUBDIRS = \
#	. \
#	test

noinst_DATA = javac-cfg

jardir = @SIPX_JAVADIR@/sipXconfig
jar_DATA =
	$(cfg_JAR)

cfg_JAR = openfire-service.jar

$(cfg_JAR) :
	jar -cf $@ -C $(JAVAROOT) .

cfg_DEPS = \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig, $(cfg_PKGS))

cfg_SRC = \
	$(shell cd $(srcdir); find org -name '*.java')
