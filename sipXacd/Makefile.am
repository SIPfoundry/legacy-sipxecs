## Process this file with automake to produce Makefile.in
include config/project.am

include config/conditional_doc.am

SUBDIRS = \
	src \
	etc \
	bin \
	$(doc_SUBDIRS)

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    BUILDSTAMP \
    SVN-VERSION \
    sipxacd.spec

install-exec-hook :

install-data-hook :
