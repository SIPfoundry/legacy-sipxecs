## Process this file with automake to produce Makefile.in
include config/project.am

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
	siptest \
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
    @PACKAGE@-rpmlintrc \
    sipxtacklib.spec


DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
