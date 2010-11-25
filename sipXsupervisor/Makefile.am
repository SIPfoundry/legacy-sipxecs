## Process this file with automake to produce Makefile.in
include config/project.am

include config/conditional_doc.am

SUBDIRS = . \
	 bin \
    src \
    etc \
    meta \
    $(doc_SUBDIRS)

EXTRA_DIST = \
    BUILDSTAMP \
    SVN-VERSION \
    CONTRIBUTORS \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipX-config.in \
    config/svn-version \
    config/Doxyfile.in \
    sipxsupervisor.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)

install-data-hook: \
	$(DESTDIR)$(SIPX_VARDIR)/process-state \
	$(DESTDIR)$(SIPX_VARDIR)/process-cfgver

$(DESTDIR)$(SIPX_VARDIR)/process-state:
	test -d $(DESTDIR)$(SIPX_VARDIR)/process-state \
	|| $(INSTALL) -d $(DESTDIR)$(SIPX_VARDIR)/process-state

$(DESTDIR)$(SIPX_VARDIR)/process-cfgver:
	test -d $(DESTDIR)$(SIPX_VARDIR)/process-cfgver \
	|| $(INSTALL) -d $(DESTDIR)$(SIPX_VARDIR)/process-cfgver
