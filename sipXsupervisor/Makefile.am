include config/project.am

SUBDIRS = . \
	bin \
	src \
	etc \
	meta

install-data-hook: \
	$(DESTDIR)$(SIPX_VARDIR)/process-state \
	$(DESTDIR)$(SIPX_VARDIR)/process-cfgver

$(DESTDIR)$(SIPX_VARDIR)/process-state:
	test -d $(DESTDIR)$(SIPX_VARDIR)/process-state \
	|| $(INSTALL) -d $(DESTDIR)$(SIPX_VARDIR)/process-state

$(DESTDIR)$(SIPX_VARDIR)/process-cfgver:
	test -d $(DESTDIR)$(SIPX_VARDIR)/process-cfgver \
	|| $(INSTALL) -d $(DESTDIR)$(SIPX_VARDIR)/process-cfgver
