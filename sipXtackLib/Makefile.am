## Process this file with automake to produce Makefile.in
include config/project.am

if SIPVIEWER
SIPVIEWER_OPT = sipviewer
endif

if EXAMPLES
EXAMPLES_OPT = examples
endif

include config/conditional_doc.am

SUBDIRS = \
	. \
	include \
	src \
	bin \
	meta \
	examples \
	dialogwatch \
	syslog2siptrace \
	siplog2siptrace \
	siptest \
	$(SIPVIEWER_OPT) \
	$(EXAMPLES_OPT) \
	$(doc_SUBDIRS)

RPMS = $(DEFAULT_RPMS) $(DEBUG_RPM)

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
    ant-targets.xml \
    ant-targets.am \
    sipxtacklib.spec \
    lib/jdom-b10.jar \
    lib/junit.jar \
    config.properties.in \
    syslog2siptrace/siptrace-merge

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
