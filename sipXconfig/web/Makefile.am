include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

SUBDIRS = \
	. \
	test

EXTRA_DIST = \
	$(gwt_SRC) \
	context \
	packages \
	$(web_SRC) \
	$(addprefix src/,$(web_RESOURCES))

noinst_DATA = javac-wsdl java-gwt javac-web 

# W S D L
WSDL = $(web_SRCDIR)/org/sipfoundry/sipxconfig/api/sipxconfig.wsdl
wsdl_JAVAROOT = $(web_JAVAROOT)
wsdl_SRCDIR = wsdl.src
wsdl_SRC = $(shell find $(wsdl_SRCDIR) -name '*.java')
wsdl_DEPS = $(call JavaDep,@SIPX_JAVADIR@/sipXcommons,axis)

javac-wsdl : wsdl-src

wsdl-src : $(WSDL)
	test -d $(wsdl_SRCDIR) || mkdir -p $(wsdl_SRCDIR)
	$(call JavaCommand,wsdl) org.apache.axis.wsdl.WSDL2Java \
	  --server-side \
	  --noWrapped \
	  --verbose \
          --NStoPkg http://www.sipfoundry.org/2007/08/21/ConfigService=org.sipfoundry.sipxconfig.api \
	  --output $(wsdl_SRCDIR) \
	  $(WSDL)
	find $(wsdl_SRCDIR) -name '*BindingImpl.java' -exec rm {} \;
	@echo timestamp > $@

# W S D D
wsdd_DEPS = \
	$(wsdl_DEPS) \
	$(web_JAVAROOT)

WSDD = $(war_ROOT)/WEB-INF/server-config.wsdd
WSDD_XSLT = $(top_srcdir)/meta/server-config.xsl

# WSDD doesn't validate, schema does not resolve among 
#  other things, so removing erroneous referance to namespaces.
# I have posted email to ant project to get feedback on this
# http://thread.gmane.org/gmane.comp.jakarta.ant.user/34470
$(WSDD) : javac-wsdl $(WSDL) $(WSDD_XSLT)
	test -d $(dir $@) || mkdir -p $(dir $@)
	$(call JavaCommand,wsdd) org.apache.axis.utils.Admin server $(wsdl_SRCDIR)/org/sipfoundry/sipxconfig/api/deploy.wsdd
	sed -e 's/<deployment.*>/<deployment>/g' server-config.wsdd > server-config-no-spring.wsdd.dirty
	$(XSLTPROC) -o server-config.wsdd.dirty $(WSDD_XSLT) server-config-no-spring.wsdd.dirty
	sed -e 's/xmlns=""//g' server-config.wsdd.dirty > $@

# G W T
gwt_PKGS = \
	gwt-user \
	gwt-dev \
	smartgwt \
	smartgwt-skins

gwt_SRC = $(shell cd $(srcdir); find gwt -type f) 

gwt_DEPS = \
	$(srcdir)/gwt/src \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons $(top_srcdir)/lib,$(gwt_PKGS))

# preserved from ant port, probably true on some systems but not verified post port
gwt_JAVACFLAGS = -Xmx512m -Xms256m

java-gwt : $(gwt_SRC)
	$(call JavaCommand,gwt) com.google.gwt.dev.Compiler \
		-style PRETTY \
		-war \
		out/gwt \
		org.sipfoundry.sipxconfig.userportal.user_phonebook_search \
		org.sipfoundry.sipxconfig.userportal.contact_information
	@echo timestamp > $@

	mv out/gwt/org.sipfoundry.sipxconfig.userportal.user_phonebook_search/org.sipfoundry.sipxconfig.userportal.user_phonebook_search.nocache.js out/gwt/org.sipfoundry.sipxconfig.userportal.user_phonebook_search/nocache.js
	mv out/gwt/org.sipfoundry.sipxconfig.userportal.contact_information/org.sipfoundry.sipxconfig.userportal.contact_information.nocache.js out/gwt/org.sipfoundry.sipxconfig.userportal.contact_information/nocache.js

DART_SRC = $(shell cd $(srcdir)/context/WEB-INF; find . -name '*.dart')
DART_PKGS = $(shell cd $(srcdir); find packages -name '*.js')

java-dart : $(addprefix $(web_JAVAROOT)/,$(DART_SRC:.dart=.js) $(DART_PKGS))

$(addprefix $(web_JAVAROOT)/,$(DART_PKGS)) : $(DART_PKGS)
	test -d $(dir $@) || mkdir -p $(dir $@)
	cp $< $@

$(addprefix $(web_JAVAROOT)/,$(DART_SRC:.dart=.js)) : $(web_JAVAROOT)/%.js : context/WEB-INF/%.dart
	test -d $(dir $@) || mkdir -p $(dir $@)
	$(DART_HOME)/bin/dart2js -o$@ $<

# W E B
web_COMPILE_PKGS = \
	$(web_PKGS) \
	junit \
	dbunit \
	p6spy \
	junit \
	easymock \
	easymockclassextension \
	spring-mock

# use the compiled output dirs when possible in case jar files have not been updated
web_DEPS = \
	$(neoconf_SRCDIR) \
	$(neoconf_JAVAROOT) \
	$(web_SRCDIR) \
	$(web_JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons $(top_srcdir)/lib,$(web_COMPILE_PKGS)) \
	@SIPX_JAVADIR@/sipXconfig/agent-bindings.jar \
	@SIPX_JAVADIR@/sipXconfig/cdr-bindings.jar

# classes from wsdl compilation are refererence in web project source so ensure this is
# done first
javac-web : javac-wsdl

web_SRC = $(shell cd $(srcdir); find src -type f -name '*.java')
web_RESOURCES = $(shell cd $(srcdir)/src; find . -type f \( \
	  -name '*.png' \
	  -o -name '*.gif' \
	  -o -name '*.jpg' \
	  -o -name '*.css' \
	  -o -name '*.xml' \
	  -o -name '*.vm' \
	  -o -name '*.wsdl' \
	  -o -name '*.properties' \
	  -o -name '*.license' \
	  -o -name '*.js' \
	  -o -name '*.ico' \
	\))

if TEST_MODE
context_TESTPAGE = 
else
context_TESTPAGE = \) -a \( -not -name 'TestPage.*'
endif

context_SRC = $(shell cd $(srcdir)/context; find . -type f \( \
	  -name '*.png' \
	  -o -name '*.gif' \
	  -o -name '*.application' \
	  -o -name '*.jpg' \
	  -o -name '*.page' \
	  -o -name '*.jwc' \
	  -o -name '*.properties' \
	  -o -name '*.html' \
	  -o -name '*.script' \
	  -o -name '*.xml' \
	  $(context_TESTPAGE) \
	\))

# W A R
wardir = @SIPX_JAVADIR@/sipXconfig
war_DATA = sipxconfig.war sipxconfig-web.jar

sipxconfig-web.jar : javac-web java-dart javac-wsdl $(addprefix src/,$(web_RESOURCES)) $(neoconf_JAR)
	jar cf $@ \
		$(call JarInclude,$(web_JAVAROOT),.) \
		$(call JarInclude,$(srcdir)/src,$(web_RESOURCES))

# create war, then update war.  There's a 'bug' in jar command will create 2 separate ./WEB-INF entries
# in jar file if listed twice. 
sipxconfig.war : java-gwt $(WSDD) $(addprefix context/,$(context_SRC))
	jar cf $@.tmp $(call JarInclude,$(war_ROOT),.)
	cd $(srcdir)/context; jar uf $(abspath $@.tmp) $(context_SRC)		
	mv $@.tmp $@

style_SRC = $(filter-out src/com/%,$(web_SRC))
style_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons $(top_srcdir)/lib,$(web_PKGS))

precommit : style validate-xml

# Not validating
#  src/org/sipfoundry/sipxconfig/site/tapestry.xml 
# because it makes heavy use of SGML's ENTITY and needs to be ported to Schema
#  see http://www.w3.org/TR/xmlschema-0/#usingEntities
SPRING_XML = \
	src/org/sipfoundry/sipxconfig/rest/rest.beans.xml \
	src/org/sipfoundry/sipxconfig/site/about/about.beans.xml \
	test/sipxplugin.beans.xml

validate-xml : $(SPRING_XML)
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^

clean :
	$(call CleanDirs,\
		$(wsdl_SRCDIR) \
		$(wsdl_JAVAROOT) \
		$(war_ROOT) \
		$(web_JAVAROOT) \
		$(wsdl_SRCDIR) \
	)
	$(call CleanFiles,\
		sipxconfig.jar \
		sipxconfig-web.jar \
		sipxconfig.war \
	    javac-web \
	    java-gwt \
	    java-dart \
		wsdl-src \
		$(WSDD) \
		server-config.wsdd.dirty \
		server-config.wsdd \
		server-config-no-spring.wsdd.dirty \
	)	
