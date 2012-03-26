include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(event_SRC) \
	$(event_RESOURCES)
	
noinst_DATA = \
	javac-event
	
jardir = @SIPX_JAVADIR@/sipXevent
JAR_FILE = sipXevent.jar
jar_DATA = $(JAR_FILE)

event_RESOURCES = \
    server.beans.xml
                      
event_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(event_PKGS))

event_SRC = \
	$(shell cd $(srcdir); find org -name '*.java')
	  
$(JAR_FILE) : javac-event $(event_RESOURCES)	
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(event_RESOURCES))	  