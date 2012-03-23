include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(websocket_SRC) \
	$(websocket_RESOURCES)
	
noinst_DATA = \
	javac-websocket
	
jardir = @SIPX_JAVADIR@/sipXwebsocket
JAR_FILE = sipXwebsocket.jar
jar_DATA = $(JAR_FILE)

websocket_RESOURCES = \
    server.beans.xml
                      
websocket_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(websocket_PKGS))

websocket_SRC = \
	$(shell cd $(srcdir); find org -name '*.java')
	  
$(JAR_FILE) : javac-websocket $(websocket_RESOURCES)	
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(websocket_RESOURCES))	  