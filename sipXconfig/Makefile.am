include config/project.am

# Append to global search/replace with _very_ project specific search/replaces
edit = $(LocalizeSipXconfig) \
    -e 's,@OPENSSL\@,@OPENSSL@,g' \
    -e 's,@CLASSPATH_BUILD\@,@CLASSPATH_BUILD@,g' \
    -e 's,@CLASSPATH_RUN\@,@CLASSPATH_RUN@,g' \
    -e 's,@CLOVER_JAR\@,@CLOVER_JAR@,g' \
    -e 's,@MRTG_ENABLED\@,@MRTG_ENABLED@,g' \
    -e 's,@RUBY_BINDINGS_ENABLED\@,@RUBY_BINDINGS_ENABLED@,g' \
    -e 's,@RUBY\@,@RUBY@,g' \
    -e 's,@WSDL2RUBY\@,@WSDL2RUBY@,g' \
    -e 's,@SIPX_PAGEMUSICDIR\@,@SIPX_PAGEMUSICDIR@,g' \
    -e 's,@SIPFOUNDRY_ORG\@,@SIPFOUNDRY_ORG@,g' \
    -e 's,@SIPX_DOWNLOADDIR\@,@SIPX_DOWNLOADDIR@,g' \
    -e 's,@MRTG_BINARY\@,@MRTG_BINARY@,g' \
    -e 's,@YUM_EXISTS\@,@YUM_EXISTS@,g'

RPMS = \
	$(RPM) \
	$(DEST_RPM)/@PACKAGE@-snmp-$(SIPX_VERSION).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/@PACKAGE@-mrtg-$(SIPX_VERSION).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/@PACKAGE@-tftp-$(SIPX_VERSION).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/@PACKAGE@-ftp-$(SIPX_VERSION).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/@PACKAGE@-agent-$(SIPX_VERSION).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/@PACKAGE@-report-$(SIPX_VERSION).@RPM_TARGET_ARCH@.rpm

if REPORTS
REPORTS_OPT = report
endif

if AGENT
AGENT_OPT = agent
endif

if MRTG
MRTG_OPT = mrtg
endif

SUBDIRS = \
	. \
	bin \
	neoconf \
	gwt \
	web \
	meta \
	plugins \
	$(REPORTS_OPT)\
	$(AGENT_OPT)\
	$(MRTG_OPT)

install-exec-hook :
	$(ANT_CMD) $(ANT_FLAGS) remove-jars

precommit :
	$(ANT_CMD) $(ANT_FLAGS) precommit

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    ant-targets.xml \
    build.xml \
    dependencies.xml \
    subant-targets.xml \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
    LICENSE \
    NOTICE \
    build.properties.in \
    lib.properties \
    neoconf \
    gwt \
    sipxconfig.spec \
    doc \
    web \
    meta \
    agent \
    report \
    mrtg \
    lib

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = build.properties

build.properties : build.properties.in Makefile _top.build.dir
	@echo "Localize $< -> $@"
	@$(edit) $(srcdir)/build.properties.in > $(top_builddir)/build.properties

# This sets the default build dir to the last directory that called
# configure shell script.  This allows 'ant' to be called from any directory
# target name is not a file to force this make everytime
_top.build.dir :
	echo "top.build.dir="`pwd` > $(top_srcdir)/top.build.dir

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
