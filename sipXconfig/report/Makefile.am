include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(report_SRC)

reportdir = @SIPX_VARDIR@/reports

report_SRC = $(shell cd $(srcdir); find src -name '*.jrxml')
report_OUT = $(addprefix $(JAVAROOT)/,$(notdir $(report_SRC:.jrxml=.jasper)))
report_DATA = $(report_OUT)

report_PKGS = \
	ant \
	ant-launcher \
	commons-digester \
	commons-logging \
	commons-collections \
	commons-beanutils \
	ant-apache-regexp \
	ant-apache-oro \
	jasperreports \
	regexp \
	jasper-compiler

report_DEPS = $(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(report_PKGS))

CLEANFILES = jasper-build.xml $(report_OUT)

jasper-build.xml : Makefile
	echo "$$jasper_report_compiler" > $@

$(report_OUT) : jasper-build.xml $(report_SRC)
# force a whole new compilation because jasper report compiler only builds if
# it thinks it needs to.  we already have dependency management here and don't
# want that 	
	! test -d $(JAVAROOT) || rm -rf $(JAVAROOT)
	mkdir -p $(JAVAROOT)
	$(call JavaCommand,report) org.apache.tools.ant.launch.Launcher -f jasper-build.xml
	touch $(report_OUT)

# There is no way to compile jasper reports from CLI. Use mini-ant project to compile. Alternative
# is to write a little java program, but that's lame because we already have ant jar available.
# from sipxcommons
define jasper_report_compiler
	<project name="jasper_compiler" default="default">
	  <target name="default">
	    <taskdef name="jrc"  classname="net.sf.jasperreports.ant.JRAntCompileTask"/>
	    <jrc compiler="net.sf.jasperreports.engine.design.JRJavacCompiler"
	      srcdir="$(srcdir)/src" destdir="$(JAVAROOT)">
	    </jrc>    
	  </target>
	</project>
endef
export jasper_report_compiler
