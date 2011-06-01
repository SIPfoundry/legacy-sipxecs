include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

noinst_DATA = javac-sipviewer
EXTRA_DIST = \
	$(sipviewer_SRC)

jardir = @SIPX_JAVADIR@/sipXviewer
jar_DATA = $(JAR_FILE)
JAR_FILE = sipviewer.jar

$(JAR_FILE) : ViewerManifest.txt javac-sipviewer
	jar cfm $@ ViewerManifest.txt -C $(JAVAROOT) .

sipviewer_SRC = $(shell cd $(srcdir); find com -type f -name '*.java')

sipviewer_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,jdom)

ViewerManifest.txt : Makefile
	echo "$$viewer_manifest" > $@

define viewer_manifest
$(manifest_body)
Class-Path: jdom.jar
Created-By: 1.6.0_14 (Sun Microsystems Inc.)
Main-Class: com.pingtel.sipviewer.SIPViewer
endef
export viewer_manifest
