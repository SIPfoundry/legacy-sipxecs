## Process this file with automake to produce Makefile.in
include config/project.am

include config/conditional_doc.am

SUBDIRS = . \
	 bin \
    src \
    etc \
    meta \
    $(doc_SUBDIRS)

RPMS = $(DEFAULT_RPMS) $(DEBUG_RPM)

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
	$(DESTDIR)$(localstatedir)/sipxecs/process-state \
	$(DESTDIR)$(localstatedir)/sipxecs/process-cfgver

$(DESTDIR)$(localstatedir)/sipxecs/process-state:
	test -d $(DESTDIR)$(localstatedir)/sipxecs/process-state \
	|| $(INSTALL) -d $(DESTDIR)$(localstatedir)/sipxecs/process-state

$(DESTDIR)$(localstatedir)/sipxecs/process-cfgver:
	test -d $(DESTDIR)$(localstatedir)/sipxecs/process-cfgver \
	|| $(INSTALL) -d $(DESTDIR)$(localstatedir)/sipxecs/process-cfgver
