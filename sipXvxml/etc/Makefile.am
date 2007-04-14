include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
    sipXvxml.process.xml.in \
    mediaserver-config.in.in

install-data-hook : $(DESTDIR)$(SIPX_PROCDIR)/sipXvxml.process.xml
	$(LocalizeSipXconfig) $(srcdir)/mediaserver-config.in.in > mediaserver-config.in ; \
	if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/mediaserver-config.in ; \
	then \
		echo ""; echo "Installing default $(sysconfdir)/sipxpbx/mediaserver-config.in"; \
		$(INSTALL) -D -m 644 mediaserver-config.in $(DESTDIR)$(sysconfdir)/sipxpbx/mediaserver-config.in; \
	else \
		echo ""; echo "Using existing $(sysconfdir)/sipxpbx/mediaserver-config.in "; \
	fi;
